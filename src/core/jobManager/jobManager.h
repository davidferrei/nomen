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

    void CreateJob(const DataSourceData& ds, const QString& caseFolder); // creates IndexJob for now

    int getJobsCount() const;

signals:
    void jobStarted(const QString& filePath);
    void jobFinished(const QString& filePath);
    void jobFailed(const QString& filePath, const QString& error);

private:
    QList<Job*> jobs;
    void registerJob(IndexJob* job);
};

#endif // JOBMANAGER_H
