
#include "jobManager.h"

JobManager::JobManager(QObject* parent) : QObject(parent) {}

//############################################################################################
// Create Jobs

void JobManager::CreateIndexJob(const DataSourceData& ds, const QString& caseFolder)
{
    qDebug() << "[JobManager] CreateIndexJob called for" << ds.filePath << "case:" << caseFolder;
    IndexJob* job = new IndexJob(ds, caseFolder, this);
    registerJob(job);
    job->start();
}

void JobManager::CreateIngestJob(const DataSourceData& ds, const QString& caseFolder, const QString& moduleName)
{
    qDebug() << "[JobManager] CreateIngestJob called for" << ds.filePath << "case:" << caseFolder;
    IngestJob* job = new IngestJob(ds, caseFolder, moduleName, this);
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
    qDebug() << "[JobManager] registerJob called for" << job << "file:" << job->getDSFilePath();

    connect(job, &Job::started, this, [this, job]() {
        qDebug() << "[JobManager] Received Job::started for" << job->getDSFilePath();
        emit jobStarted(job->getDSFilePath(), job->getType());
    }, Qt::QueuedConnection);

    connect(job, &Job::finished, this, [this, job]() {
        qDebug() << "[JobManager] Received Job::finished for" << job->getDSFilePath();
        emit jobFinished(job->getDSFilePath(), job->getType());
        jobs.removeOne(job);
        job->deleteLater();
    }, Qt::QueuedConnection);

    connect(job, &Job::failed, this, [this, job](const QString& error) {
        qDebug() << "[JobManager] Received Job::failed for" << job->getDSFilePath() << ":" << error;
        emit jobFailed(job->getDSFilePath(), error, job->getType());
        jobs.removeOne(job);
        job->deleteLater();
    }, Qt::QueuedConnection);
}
