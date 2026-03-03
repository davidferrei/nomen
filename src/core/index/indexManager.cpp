#include "indexManager.h"

IndexManager::IndexManager(QObject* parent) : QObject(parent)
{
    qDebug() << "Index Manager Created";
}


//############################################################################################
// Logic

void IndexManager::CreateJob(const DataSourceData& ds, const QString& caseFolder)
{
    qDebug() << "[IndexManager] Creating Job for:" << ds.filePath;

    IndexJob* job = new IndexJob(ds, caseFolder, this);
    jobs.append(job);

    // Signals
        connect(job, &IndexJob::started, this, [this, job]() {
            qDebug() << "[IndexManager] Job Started";
            emit jobStarted(job->getDSFilePath());
        });

        connect(job, &IndexJob::finished, this, [this, job]() {
            qDebug() << "[IndexManager] Job Finished";

            emit jobFinished(job->getDSFilePath());

            jobs.removeOne(job);
            job->deleteLater();
        });

        connect(job, &IndexJob::failed, this, [this, job](const QString& error) {
            qDebug() << "[IndexManager] Job Failed:" << error;

            emit jobFailed(job->getDSFilePath(), error);

            jobs.removeOne(job);
            job->deleteLater();
        });

    job->start();

}

//############################################################################################
// Setters/Getters

int IndexManager::getJobsCount() const
{
    return jobs.size();
}
