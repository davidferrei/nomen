#ifndef INDEXJOB_H
#define INDEXJOB_H

#include <QObject>
#include <QUuid>

#include "core/jobManager/job.h"

#include "indexWorker.h"


class IndexJob : public Job
{
    Q_OBJECT

public:

    IndexJob(const DataSourceData& ds, const QString& caseFolder, QObject* parent = nullptr);
    ~IndexJob();

    void start() override;

private:
    QString caseFolderPath;
    bool isRunning = false;
    bool workerSucceeded = false;
    QString workerError;

    QThread* thread = nullptr;
    IndexWorker* worker = nullptr;
};

#endif // INDEXJOB_H
