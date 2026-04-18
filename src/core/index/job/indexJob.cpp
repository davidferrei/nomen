#include "indexJob.h"

IndexJob::IndexJob(const DataSourceData& ds, const QString& casePath, QObject* parent) : Job(Job::JobType::Index, ds, parent), caseFolderPath(casePath) {}

IndexJob::~IndexJob()
{
    if (thread && thread->isRunning()) 
    {
        disconnect(thread, nullptr, this, nullptr);
        disconnect(worker, nullptr, this, nullptr);
        thread->quit();
        thread->wait();
    }
    delete worker;
    delete thread;
}

void IndexJob::start()
{
    if (isRunning)
    {
        qWarning() << "[IndexJob] Already Running for" << dataSource.filePath;
        return;
    }

    isRunning = true;

    thread = new QThread();
    QString jobId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    thread->setObjectName("IndexJob-" + jobId);

    worker = new IndexWorker(dataSource, caseFolderPath);

    worker->moveToThread(thread);

    // Signals
        connect(thread, &QThread::started, worker, &IndexWorker::run);

        connect(worker, &IndexWorker::finished, this, [this]() {
            qDebug() << "[IndexJob] Worker Finished for" << dataSource.filePath;
            workerSucceeded = true;
        });

        connect(worker, &IndexWorker::failed, this, [this](const QString& error) {
            qDebug() << "[IndexJob] Worker Failed for" << dataSource.filePath << ":" << error;
            workerError = error;
        });

        connect(worker, &IndexWorker::finished, thread, &QThread::quit);
        connect(worker, &IndexWorker::failed, thread, &QThread::quit);

        connect(thread, &QThread::finished, this, [this]() {
            qDebug() << "[IndexJob] Thread Stopped for" << dataSource.filePath;

            isRunning = false;

            worker->deleteLater();
            thread->deleteLater();
            worker = nullptr;
            thread = nullptr;

            if (workerSucceeded) {
                emit finished();
            } else {
                emit failed(workerError);
            }
        });

    qDebug() << "[IndexJob] Starting Job for" << dataSource.filePath << "in Thread" << thread->objectName();
    qDebug() << "[IndexJob] start() called on" << this << "for" << dataSource.filePath;

    emit started();

    thread->start();
}

