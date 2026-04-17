#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QObject>
#include <QList>

#include "core/jobManager/job.h"
#include "data/evidence/dataSourceData.h"
#include "core/index/job/indexJob.h"
#include "core/ingest/job/ingestJob.h"

class JobManager : public QObject
{
    Q_OBJECT
public:
    explicit JobManager(QObject* parent = nullptr);

    void CreateIndexJob(const DataSourceData& ds, const QString& caseFolder);
    void CreateIngestJob(const DataSourceData& ds, const QString& caseFolder);

    int getJobsCount() const;

signals:
    void jobStarted(const QString& filePath);
    void jobFinished(const QString& filePath);
    void jobFailed(const QString& filePath, const QString& error);

private:
    QList<Job*> jobs;
    void registerJob(Job* job);
};

#endif // JOBMANAGER_H
