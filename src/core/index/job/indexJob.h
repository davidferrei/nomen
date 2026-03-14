#ifndef INDEXJOB_H
#define INDEXJOB_H

#include <QObject>
#include <QUuid>

#include "data/evidence/dataSourceData.h"

#include "indexWorker.h"


class IndexJob : public QObject
{
    Q_OBJECT

public:
    IndexJob(const DataSourceData& ds, const QString& caseFolder, QObject* parent = nullptr);
    ~IndexJob();

    void start();

    QString getDSFilePath() const;


signals:
    void started();
    void finished();
    void failed(QString error);

private:
    DataSourceData dataSource;
    QString caseFolderPath;
    bool isRunning = false;
    bool workerSucceeded = false;
    QString workerError;

    QThread* thread = nullptr;
    IndexWorker* worker = nullptr;
};

#endif // INDEXJOB_H
