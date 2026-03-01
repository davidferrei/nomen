#ifndef INDEXJOB_H
#define INDEXJOB_H

#include <QObject>

#include "data/evidence/dataSourceData.h"


class IndexJob : public QObject
{
    Q_OBJECT

public:
    IndexJob(const DataSourceData& ds, QObject* parent = nullptr);

    void start();

signals:
    void started();
    void finished();
    void failed(QString error);

private:
    DataSourceData dataSource;
    bool isRunning = false;
};

#endif // INDEXJOB_H
