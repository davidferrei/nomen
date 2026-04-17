#ifndef INGESTJOB_H
#define INGESTJOB_H

#include "core/jobManager/job.h"
#include "data/evidence/dataSourceData.h"

#include <QThread>
#include <QUuid>
#include <QThread>

class IngestJob : public Job
{
    Q_OBJECT
public:
    IngestJob(const DataSourceData& ds, const QString& caseFolder, QObject* parent = nullptr);
    ~IngestJob();

    void start() override;

signals:
    void started();
    void finished();
    void failed(QString error);

private:
    QString caseFolderPath;
    bool isRunning = false;
    bool workerSucceeded = false;
    QString workerError;

    QThread* thread = nullptr;
    QObject* worker = nullptr;
};

#endif // INGESTJOB_H