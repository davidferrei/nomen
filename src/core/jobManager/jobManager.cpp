
#include "jobManager.h"

JobManager::JobManager(QObject* parent) : QObject(parent) {}

//############################################################################################
// Create Jobs

void JobManager::CreateIndexJob(const DataSourceData& ds, const QString& caseFolder)
{
    IndexJob* job = new IndexJob(ds, caseFolder, this);
    registerJob(job);
    job->start();
}

void JobManager::CreateIngestJob(const DataSourceData& ds, const QString& caseFolder)
{
    IngestJob* job = new IngestJob(ds, caseFolder, this);
    registerJob(job);
    job->start();
}

//############################################################################################
// Functions

int JobManager::getJobsCount() const
{
    return jobs.size();
}

void JobManager::registerJob(Job* job)
{
    jobs.append(job);

    connect(job, &Job::started, this, [this, job]() {
        emit jobStarted(job->getDSFilePath());
    });

    connect(job, &Job::finished, this, [this, job]() {
        emit jobFinished(job->getDSFilePath());
        jobs.removeOne(job);
        job->deleteLater();
    });

    connect(job, &Job::failed, this, [this, job](const QString& error) {
        emit jobFailed(job->getDSFilePath(), error);
        jobs.removeOne(job);
        job->deleteLater();
    });
}
