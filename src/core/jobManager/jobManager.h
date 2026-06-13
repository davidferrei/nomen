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
    void CreateIngestJob(const DataSourceData& ds, const QString& caseFolder, const QString& moduleName);

    int getJobsCount() const;

signals:
    void jobStarted(const QString& filePath, Job::JobType type, const QString& moduleName, const QString& dsName, qint64 dsId);
    void jobFinished(const QString& filePath, Job::JobType type, const QString& moduleName, const QString& dsName, qint64 dsId);
    void jobFailed(const QString& filePath, const QString& error, Job::JobType type, const QString& moduleName, const QString& dsName, qint64 dsId);

private:
    QList<Job*> jobs;
    void registerJob(Job* job);
};

#endif // JOBMANAGER_H
