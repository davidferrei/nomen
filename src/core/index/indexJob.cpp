#include "indexJob.h"

IndexJob::IndexJob(const DataSourceData& ds, const QString& casePath, QObject* parent) : QObject(parent), dataSource(ds), caseFolderPath(casePath) {}

void IndexJob::start()
{
    if (isRunning)
    {
        qWarning() << "[IndexJob] Already Running for" << dataSource.filePath;
        return;
    }

    isRunning = true;

    thread = new QThread(this);
    QString jobId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    thread->setObjectName("IndexJob-" + jobId);

    worker = new IndexWorker(dataSource, caseFolderPath);

    worker->moveToThread(thread);

    // Signals
        connect(thread, &QThread::started, worker, &IndexWorker::run);
        connect(worker, &IndexWorker::finished, thread, &QThread::quit);

        connect(worker, &IndexWorker::finished, this, [this]() {
            qDebug() << "[IndexJob] Worker Finished for" << dataSource.filePath;
            isRunning = false;
            emit finished();
        });

        connect(worker, &IndexWorker::failed, this, [this](const QString& error) {
            qDebug() << "[IndexJob] Worker Failed for" << dataSource.filePath << ":" << error;
            isRunning = false;
            emit failed(error);
        });

        connect(worker, &QObject::destroyed, this, [this]() {
            qDebug() << "[IndexJob] Worker Deleted for" << dataSource.filePath;
        });

        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        connect(thread, &QObject::destroyed, this, [this]() {
            qDebug() << "[IndexJob] Thread Deleted for" << dataSource.filePath;
        });

    qDebug() << "[IndexJob] Starting Job for" << dataSource.filePath << "in Thread" << thread->objectName();

    emit started();

    thread->start();
}

//############################################################################################
// Setters/Getters


QString IndexJob::getDSFilePath() const
{
    return dataSource.filePath;
}
