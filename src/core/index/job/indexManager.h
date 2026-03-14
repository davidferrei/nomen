#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H

#include <QObject>

#include "data/evidence/dataSourceData.h"

#include "indexJob.h"

class IndexManager : public QObject
{
    Q_OBJECT

public:
    IndexManager(QObject* parent = nullptr);

    void CreateJob(const DataSourceData& ds, const QString& caseFolder);

    int getJobsCount() const;

signals:
    void jobStarted(const QString& filePath);
    void jobFinished(const QString& filePath);
    void jobFailed(const QString& filePath, const QString& error);

private:
    QList<IndexJob*> jobs;
};

#endif // INDEXMANAGER_H
