#ifndef INDEXWORKER_H
#define INDEXWORKER_H

#include <QObject>
#include <QThread>

#include "data/evidence/dataSourceData.h"

#include "core/index/engine/lmdbIndexEngine.h"

class IndexWorker : public QObject
{
    Q_OBJECT

public:
    IndexWorker(const DataSourceData& ds, const QString& casePath, QObject* parent = nullptr);

public slots:
    void run();

signals:
    void finished();
    void failed(const QString& error);

private:
    DataSourceData dataSource;
    QString caseFolderPath;
};

#endif // INDEXWORKER_H
