#ifndef INGESTRESULTDBINSTANCE_H
#define INGESTRESULTDBINSTANCE_H

#include <ibpp/core/ibpp.h>

#include "core/caseDBManager/repositories/ingestResultItemRepository.h"

#include <QDebug>
#include <QObject>
#include <QString>

class IngestResultDBInstance : public QObject
{
    Q_OBJECT
public:
    explicit IngestResultDBInstance(IBPP::Database db, qint64 jobId, QObject *parent = nullptr);

    qint64 createFolderItem(const QString &name);
    qint64 createTextItem(qint64 parentId, const QString &label, const QString &value);

private:
    IngestResultItemRepository repo;
    qint64 jobId = 0;
};

#endif // INGESTRESULTDBINSTANCE_H
