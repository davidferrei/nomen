#ifndef INGESTRESULTITEMREPOSITORY_H
#define INGESTRESULTITEMREPOSITORY_H

#include <ibpp/core/ibpp.h>
#include <QList>
#include "data/ingest/ingestResultItemData.h"

class IngestResultItemRepository
{
public:
    explicit IngestResultItemRepository(IBPP::Database db);

    bool insert(IngestResultItemData &item);
    QList<IngestResultItemData> getChildrenItems(qint64 parentItemId);
    QList<IngestResultItemData> getTopLevelItemsForJob(qint64 jobId);

private:
    IBPP::Database fb_db;
    static IngestResultItemData rowToData(IBPP::Statement &st);
};

#endif // INGESTRESULTITEMREPOSITORY_H
