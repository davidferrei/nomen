#ifndef INGESTWORKER_H
#define INGESTWORKER_H

#include <QObject>
#include <QThread>

#include "data/evidence/dataSourceData.h"

#include "extensions/ingestModules/ingestModuleFactory.h"
#include "core/forensic/tsk/tskIngestModule.h"
#include "../../logging/performancemeasure.h"




class IngestWorker : public QObject
{
    Q_OBJECT

public:
    IngestWorker(const DataSourceData& ds, const QString& casePath, QObject* parent = nullptr);

public slots:
    void run();

signals:
    void finished();
    void failed(const QString &error);

private:
    DataSourceData dataSource;
    QString caseFolderPath;
    const QString moduleName = "TxtCounter";
};

#endif // INGESTWORKER_H
