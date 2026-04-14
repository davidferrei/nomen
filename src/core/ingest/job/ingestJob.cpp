#include "ingestJob.h"
#include <QDebug>
#include <QUuid>

IngestJob::IngestJob(const DataSourceData& ds, const QString& caseFolder, QObject* parent) : Job(Job::JobType::Ingest, ds, parent), caseFolderPath(caseFolder) {}

IngestJob::~IngestJob()
{
    if (thread && thread->isRunning()) 
    {
        disconnect(thread, nullptr, this, nullptr);
        thread->quit();
        thread->wait();
    }
    delete worker;
    delete thread;
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

    //TODO: Implement IngestWorker

    // Signals
        connect(thread, &QThread::started, this, [this]() {
            qDebug() << "[IngestJob] Placeholder run for" << dataSource.filePath;
            workerSucceeded = true;
            emit finished();
            thread->quit();
        });

        connect(thread, &QThread::finished, this, [this]() {
            qDebug() << "[IngestJob] Thread Stopped for" << dataSource.filePath;
            isRunning = false;
            thread = nullptr;

            //TODO: Handle Worker Failure
        });

    qDebug() << "[IngestJob] Starting Job for" << dataSource.filePath << "in Thread" << thread->objectName();

    emit started();

    thread->start();
}
