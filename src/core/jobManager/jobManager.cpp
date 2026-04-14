
#include "jobManager.h"

JobManager::JobManager(QObject* parent) : QObject(parent) {}

//############################################################################################
// Functions

void JobManager::CreateJob(const DataSourceData& ds, const QString& caseFolder)
{
    // FIXME: Add Job Creation by Type
    IndexJob* job = new IndexJob(ds, caseFolder, this);
    registerJob(job);
    job->start();
}

int JobManager::getJobsCount() const
{
    return jobs.size();
}

void JobManager::registerJob(IndexJob* job)
{
    // FIXME: Abstract IndexJob
    jobs.append(job);

    connect(job, &IndexJob::started, this, [this, job]() {
        emit jobStarted(job->getDSFilePath());
    });

    connect(job, &IndexJob::finished, this, [this, job]() {
        emit jobFinished(job->getDSFilePath());
        jobs.removeOne(job);
        job->deleteLater();
    });

    connect(job, &IndexJob::failed, this, [this, job](const QString& error) {
        emit jobFailed(job->getDSFilePath(), error);
        jobs.removeOne(job);
        job->deleteLater();
    });
}
