#include "ingestFileRepository.h"
#include <QCryptographicHash>

IngestFileRepository::IngestFileRepository(IBPP::Database db) : fb_db(db) {}

bool IngestFileRepository::insert(IngestFileData &data)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);


    //FIXME: Notes is being overwritten and not appended
    st->Prepare(
        "UPDATE OR INSERT INTO IngestFileTable (DATASOURCE_ID, PARTITION_ID, FILESYSTEM_ID, LMDB_PATH, LMDB_PATH_HASH, NOTES)"
        " VALUES (?, ?, ?, ?, ?, ?)"
        " MATCHING (DATASOURCE_ID, LMDB_PATH_HASH)"
        " RETURNING ID"
    );

    st->Set(1, static_cast<int64_t>(data.datasourceId));

    if (data.partitionId < 0)
        st->SetNull(2);
    else
        st->Set(2, static_cast<int16_t>(data.partitionId));

    if (data.filesystemId < 0)
        st->SetNull(3);
    else
        st->Set(3, static_cast<int16_t>(data.filesystemId));

    st->Set(4, data.lmdbPath.toStdString());
    st->Set(5, data.lmdbPathHash.toStdString());

    if (data.notes.isEmpty())
        st->SetNull(6);
    else
        st->Set(6, data.notes.toStdString());

    st->Execute();

    if (st->Fetch()) 
    {
        int64_t newId = 0;
        st->Get(1, newId);
        data.id = static_cast<qint64>(newId);
    }

    tr->Commit();
    return true;
}

QPair<bool, IngestFileData> IngestFileRepository::getById(qint64 id)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "SELECT ID, DATASOURCE_ID, PARTITION_ID, FILESYSTEM_ID, LMDB_PATH, LMDB_PATH_HASH, NOTES"
        " FROM IngestFileTable"
        " WHERE ID = ?"
    );
    st->Set(1, static_cast<int64_t>(id));
    st->Execute();

    IngestFileData data;
    if (st->Fetch())
    {
        int64_t tmp64 = 0;
        st->Get(1, tmp64);
        data.id = static_cast<qint64>(tmp64);

        if (!st->IsNull(2))
        { 
            st->Get(2, tmp64); 
            data.datasourceId = static_cast<qint64>(tmp64); 
        }
        if (!st->IsNull(3))
        { 
            st->Get(3, tmp64); 
            data.partitionId = static_cast<qint64>(tmp64); 
        }
        if (!st->IsNull(4)) 
        { 
            st->Get(4, tmp64); 
            data.filesystemId = static_cast<qint64>(tmp64); 
        }

        std::string s1;
        st->Get(5, s1);
        data.lmdbPath = QString::fromStdString(s1);
    
        std::string s2;
        st->Get(6, s2);
        data.lmdbPathHash = QString::fromStdString(s2);
    

        if (!st->IsNull(7)) 
        {
            std::string s; st->Get(7, s); data.notes = QString::fromStdString(s);
        }

        tr->Commit();
        return qMakePair(true, data);
    }

    tr->Commit();
    qWarning() << "[IngestFileRepository] getById: No IngestFile Found with ID" << id;
    return qMakePair(false, data);
}
