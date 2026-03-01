#include "indexJob.h"

IndexJob::IndexJob(const DataSourceData& ds, QObject* parent) : QObject(parent), dataSource(ds) {}

void IndexJob::start()
{
    if (isRunning)
    {
        qWarning() << "[IndexJob] Already Running for" << dataSource.filePath;
        return;
    }

    isRunning = true;

    qDebug() << "[IndexJob] Started:" << dataSource.filePath;
    emit started();



    qDebug() << "[IndexJob] Finished:" << dataSource.filePath;
    emit finished();

    isRunning = false;
}
