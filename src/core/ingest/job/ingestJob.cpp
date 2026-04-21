#include "ingestJob.h"
#include "ingestWorker.h"


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

    worker = new IngestWorker(dataSource, caseFolderPath);

    worker->moveToThread(thread);

    // Signals
    connect(thread, &QThread::started, worker, &IngestWorker::run);

    connect(worker, &IngestWorker::finished, this, [this]() {
        qDebug() << "[IngestJob] Worker Finished for" << dataSource.filePath;
        workerSucceeded = true;
    });

    connect(worker, &IngestWorker::failed, this, [this](const QString& error) {
        qDebug() << "[IngestJob] Worker Failed for" << dataSource.filePath << ":" << error;
        workerError = error;
    });

    connect(worker, &IngestWorker::finished, thread, &QThread::quit);
    connect(worker, &IngestWorker::failed, thread, &QThread::quit);

    connect(thread, &QThread::finished, this, [this]() {
        qDebug() << "[IngestJob] Thread Stopped for" << dataSource.filePath;

        isRunning = false;

        worker->deleteLater();
        thread->deleteLater();
        worker = nullptr;
        thread = nullptr;

        if (workerSucceeded) 
        {
            emit finished();
        } 
        else 
        {
            emit failed(workerError);
        }
    });

    qDebug() << "[IngestJob] Starting Job for" << dataSource.filePath << "in Thread" << thread->objectName();

    emit started();

    thread->start();
}
