#ifndef INGESTJOB_H
#define INGESTJOB_H

#include "core/jobManager/job.h"
#include "data/evidence/dataSourceData.h"

#include <QThread>
#include <QUuid>
#include <QThread>

#include "ingestWorker.h"

class IngestJob : public Job
{
    Q_OBJECT
public:
    IngestJob(const DataSourceData& ds, const QString& caseFolder, const QString& moduleName, QObject* parent = nullptr);
    ~IngestJob();

    void start() override;

private:
    QString caseFolderPath;
    bool isRunning = false;
    bool workerSucceeded = false;
    QString workerError;

    QThread* thread = nullptr;
    IngestWorker* worker = nullptr;
    QString moduleName;
};

#endif // INGESTJOB_H