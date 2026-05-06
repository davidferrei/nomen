#ifndef INGESTRESULTDBINSTANCE_H
#define INGESTRESULTDBINSTANCE_H

#include <ibpp/core/ibpp.h>

#include "core/caseDBManager/repositories/ingestResultItemRepository.h"
#include "core/caseDBManager/repositories/ingestFileRepository.h"

#include <QDebug>
#include <QObject>
#include <QString>
#include <QCryptographicHash>


class IngestResultDBInstance : public QObject
{
    Q_OBJECT
public:
    explicit IngestResultDBInstance(IBPP::Database db, qint64 jobId, QObject *parent = nullptr);


    //TODO: Create Batch Insert Func
    
    // Ingest Result Item Repo
    qint64 createFolderItem(const QString &name);
    qint64 createFolderItem(qint64 parentId, const QString &name);
    qint64 createTextItem(qint64 parentId, const QString &label, const QString &value);

    // Ingest File Repo
    qint64 createFileItem(qint64 datasourceId, qint64 partitionId, qint64 filesystemId, const QString &lmdbPath, const QString &notes = QString(), qint64 parentItemId = -1);


private:
    IngestResultItemRepository itemRepo;
    IngestFileRepository fileRepo;
    qint64 jobId = 0;
};

#endif // INGESTRESULTDBINSTANCE_H
