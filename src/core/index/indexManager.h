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

    void CreateJob(const DataSourceData& ds);

signals:
    // void JobStarted(const QString& filePath);
    // void JobFinished(const QString& filePath);

private:
    QList<IndexJob*> jobs;
};

#endif // INDEXMANAGER_H
