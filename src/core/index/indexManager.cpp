#include "indexManager.h"

IndexManager::IndexManager(QObject* parent) : QObject(parent)
{
    qDebug() << "Index Manager Created";
}


void IndexManager::CreateJob(const DataSourceData& ds)
{
    qDebug() << "[IndexManager] Creating Job for:" << ds.filePath;

    IndexJob* job = new IndexJob(ds, this);
    jobs.append(job);

    // Signals

    connect(job, &IndexJob::started, this, [job]() {
        qDebug() << "[IndexManager] Job Started";
    });

    connect(job, &IndexJob::finished, this, [this, job]() {
        qDebug() << "[IndexManager] Job Finished";

        jobs.removeOne(job);
        job->deleteLater();
    });

    connect(job, &IndexJob::failed, this, [this, job](const QString& error) {
        qDebug() << "[IndexManager] Job Failed:" << error;

        jobs.removeOne(job);
        job->deleteLater();
    });

    // emit JobStarted(ds.filePath);


    job->start();


    // emit JobFinished(ds.filePath);
}
