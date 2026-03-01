#include "indexJob.h"

IndexJob::IndexJob(const DataSourceData& ds, QObject* parent) : QObject(parent), dataSource(ds) {}

// void IndexJob::start()
// {
//     if (isRunning)
//     {
//         qWarning() << "[IndexJob] Already Running for" << dataSource.filePath;
//         return;
//     }

//     isRunning = true;

//     thread = new QThread(this);
//     worker = new IndexWorker(dataSource);
//     QString jobId = QUuid::createUuid().toString(QUuid::WithoutBraces);

//     worker->moveToThread(thread);

//     connect(thread, &QThread::started, worker, &IndexWorker::run);
//     connect(worker, &IndexWorker::finished, thread, &QThread::quit);
//     connect(worker, &IndexWorker::finished, this, &IndexJob::finished);
//     connect(worker, &IndexWorker::finished, worker, &QObject::deleteLater);
//     connect(thread, &QThread::finished, thread, &QObject::deleteLater);

//     qDebug() << "[IndexJob] Job Started for:" << dataSource.filePath << "With Thread ID:" << jobId;
//     emit started();

//     thread->start();
//     thread->setObjectName("IndexJob-" + jobId);


//     #if defined(Q_OS_WIN)
//         //TODO: Test This
//         ::SetThreadDescription(reinterpret_cast<HANDLE>(thread->currentThreadId()), (LPCWSTR)QString("IndexJob").utf16());
//     #elif defined(Q_OS_UNIX)
//         pthread_setname_np(pthread_self(), "IndexJob");
//     #endif
// }

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

    worker = new IndexWorker(dataSource);

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
