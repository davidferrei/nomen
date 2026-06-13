#ifndef INGESTWORKER_H
#define INGESTWORKER_H

#include <QObject>
#include <QThread>

#include "data/evidence/dataSourceData.h"
#include "data/ingest/ingestJobHistoryData.h"


#include "core/extensions/ingestModuleFactory.h"
#include "core/extensions/tskIngestModuleInterface.h"
#include "core/caseDBManager/caseDBThreadInstance.h"
#include "core/ingest/repo/ingestResultDBInstance.h"


#include "../../logging/performancemeasure.h"
#include "../../logging/logger.h"


class IngestWorker : public QObject
{
    Q_OBJECT

public:
    IngestWorker(const DataSourceData& ds, const QString& casePath, const QString& moduleName, QObject* parent = nullptr);

public slots:
    void run();

signals:
    void finished();
    void failed(const QString &error);

private:
    DataSourceData dataSource;
    QString caseFolderPath;
    QString moduleName;
};

#endif // INGESTWORKER_H
