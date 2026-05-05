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

    tr->Commit();
    return true;
}
