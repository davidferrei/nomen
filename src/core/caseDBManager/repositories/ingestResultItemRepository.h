#ifndef INGESTRESULTITEMREPOSITORY_H
#define INGESTRESULTITEMREPOSITORY_H

#include <ibpp/core/ibpp.h>


#include "data/ingest/ingestResultItemData.h"

#include <QPair>
#include <QList>
#include <QDebug>

class IngestResultItemRepository
{
public:
    explicit IngestResultItemRepository(IBPP::Database db);

    bool insert(IngestResultItemData &item);
    QList<IngestResultItemData> getChildrenItems(qint64 parentItemId);
    QList<IngestResultItemData> getTopLevelItemsForJob(qint64 jobId);
    QPair<bool, IngestResultItemData> getById(qint64 id);

private:
    IBPP::Database fb_db;
    static IngestResultItemData rowToData(IBPP::Statement &st);
};

#endif // INGESTRESULTITEMREPOSITORY_H
