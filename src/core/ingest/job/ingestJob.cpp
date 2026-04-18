#include "ingestJob.h"


IngestJob::IngestJob(const DataSourceData& ds, const QString& caseFolder, QObject* parent) : Job(Job::JobType::Ingest, ds, parent), caseFolderPath(caseFolder) {}

IngestJob::~IngestJob()
{

    if (thread && thread->isRunning())
    {
        disconnect(thread, nullptr, this, nullptr);
        thread->quit();
        thread->wait();
        delete worker;
        delete thread;
    }
}

//############################################################################################
// Functions

void IngestJob::start()
{
    if (isRunning)
    {
        qWarning() << "[IngestJob] Already Running for" << dataSource.filePath;
        return;
    }

    isRunning = true;

    thread = new QThread();
    QString jobId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    thread->setObjectName("IngestJob-" + jobId);

    qDebug() << "[IngestJob] start() called on" << this << "for" << dataSource.filePath << "thread name:" << thread->objectName();

    //TODO: Implement IngestWorker

    worker = new QObject();
    worker->moveToThread(thread);

    // Signals
    connect(thread, &QThread::started, worker, [this]() {
        qDebug() << "[IngestJob] Placeholder Worker run for" << dataSource.filePath;
        QThread::sleep(10);
        workerSucceeded = true;
        QThread::currentThread()->quit();
    });

    connect(thread, &QThread::finished, worker, &QObject::deleteLater);

    connect(thread, &QThread::finished, this, [this]() {
        qDebug() << "[IngestJob] Thread Finished for" << dataSource.filePath;
        isRunning = false;
        worker = nullptr;
        QThread* t = thread;
        thread = nullptr;
        t->deleteLater();

        if (workerSucceeded)
            emit finished();
        else
            emit failed(workerError);
    });

    qDebug() << "[IngestJob] Starting Job for" << dataSource.filePath << "in Thread" << thread->objectName();

    emit started();

    thread->start();
}
