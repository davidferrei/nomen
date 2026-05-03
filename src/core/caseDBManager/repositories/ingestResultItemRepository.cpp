#include "ingestResultItemRepository.h"

IngestResultItemRepository::IngestResultItemRepository(IBPP::Database db) : fb_db(db) {}

bool IngestResultItemRepository::insert(IngestResultItemData &item)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "INSERT INTO IngestResultItems"
        " (JOB_ID, PARENT_ITEM_ID, ITEM_TYPE, ITEM_NAME,"
        "  DATASOURCE_ID, PARTITION_ID, FILESYSTEM_ID, LMDB_PATH)"
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
    );

    st->Set(1, static_cast<int64_t>(item.jobId));

    if (item.parentItemId < 0)
        st->SetNull(2);
    else
        st->Set(2, static_cast<int64_t>(item.parentItemId));

    st->Set(3, static_cast<int16_t>(static_cast<int>(item.itemType)));
    st->Set(4, item.itemName.isEmpty() ? "" : item.itemName.toStdString());

    if (item.datasourceId < 0)
        st->SetNull(5);
    else
        st->Set(5, static_cast<int64_t>(item.datasourceId));

    if (item.partitionId < 0)
        st->SetNull(6);
    else
        st->Set(6, static_cast<int16_t>(item.partitionId));

    if (item.filesystemId < 0)
        st->SetNull(7);
    else
        st->Set(7, static_cast<int16_t>(item.filesystemId));

    if (item.lmdbPath.isEmpty())
        st->SetNull(8);
    else
        st->Set(8, item.lmdbPath.toStdString());

    st->Execute();

    st->Prepare("SELECT GEN_ID(GEN_INGESTRESULT_ID, 0) FROM RDB$DATABASE");
    st->Execute();
    if (st->Fetch())
    {
        int64_t newId = 0;
        st->Get(1, newId);
        item.id = static_cast<qint64>(newId);
    }

    tr->Commit();
    return true;
}

QList<IngestResultItemData> IngestResultItemRepository::getChildrenItems(qint64 parentItemId)
{
    QList<IngestResultItemData> result;

    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "SELECT ID, JOB_ID, PARENT_ITEM_ID, ITEM_TYPE, ITEM_NAME,"
        " DATASOURCE_ID, PARTITION_ID, FILESYSTEM_ID, LMDB_PATH"
        " FROM IngestResultItems"
        " WHERE PARENT_ITEM_ID = ?"
    );
    st->Set(1, static_cast<int64_t>(parentItemId));
    st->Execute();

    while (st->Fetch())
        result.append(rowToData(st));

    tr->Commit();
    return result;
}

QList<IngestResultItemData> IngestResultItemRepository::getTopLevelItemsForJob(qint64 jobId)
{
    QList<IngestResultItemData> result;

    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "SELECT ID, JOB_ID, PARENT_ITEM_ID, ITEM_TYPE, ITEM_NAME,"
        " DATASOURCE_ID, PARTITION_ID, FILESYSTEM_ID, LMDB_PATH"
        " FROM IngestResultItems"
        " WHERE JOB_ID = ? AND PARENT_ITEM_ID IS NULL"
    );
    st->Set(1, static_cast<int64_t>(jobId));
    st->Execute();

    while (st->Fetch())
        result.append(rowToData(st));

    tr->Commit();
    return result;
}

IngestResultItemData IngestResultItemRepository::rowToData(IBPP::Statement &st)
{
    IngestResultItemData item;
    int64_t tmp64 = 0;

    st->Get(1, tmp64);
    item.id = static_cast<qint64>(tmp64);

    st->Get(2, tmp64);
    item.jobId = static_cast<qint64>(tmp64);

    if (!st->IsNull(3))
    {
        st->Get(3, tmp64);
        item.parentItemId = static_cast<qint64>(tmp64);
    }

    int16_t itype = 0;
    st->Get(4, itype);
    item.itemType = static_cast<IngestResultItemData::ItemType>(itype);

    {
        std::string s;
        st->Get(5, s);
        item.itemName = QString::fromStdString(s);
    }

    if (!st->IsNull(6))
    {
        st->Get(6, tmp64);
        item.datasourceId = static_cast<qint64>(tmp64);
    }

    if (!st->IsNull(7))
    {
        int16_t p = 0;
        st->Get(7, p);
        item.partitionId = p;
    }

    if (!st->IsNull(8))
    {
        int16_t f = 0;
        st->Get(8, f);
        item.filesystemId = f;
    }

    if (!st->IsNull(9))
    {
        std::string s;
        st->Get(9, s);
        item.lmdbPath = QString::fromStdString(s);
    }

    return item;
}
