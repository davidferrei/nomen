#include "datasourceRepository.h"

DataSourceRepository::DataSourceRepository(IBPP::Database db) : fb_db(db) {}

bool DataSourceRepository::insert(const DataSourceData& ds)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "INSERT INTO DATASOURCE "
        "(NAME, DISKSIZE, SECTORSIZE, HASH, HASH_TYPE, FORMAT, "
        "FILEPATH, TIMEZONE, NOTES, INDEXED) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );

    st->Set(1, ds.name.isEmpty() ? "" : ds.name.toStdString());
    st->Set(2, static_cast<int64_t>(0));
    st->Set(3, static_cast<int32_t>(0));
    st->Set(4, ds.hash.isEmpty() ? "" : ds.hash.toStdString());
    st->Set(5, ds.hashType.isEmpty() ? "" : ds.hashType.toStdString());
    st->Set(6, ds.format.isEmpty() ? "" : ds.format.toStdString());
    st->Set(7, ds.filePath.isEmpty() ? "" : ds.filePath.toStdString());
    st->Set(8, ds.timeZone.isEmpty() ? "" : ds.timeZone.toStdString());
    st->Set(9, ds.notes.isEmpty() ? "" : ds.notes.toStdString());
    st->Set(10, static_cast<int16_t>(ds.indexed ? 1 : 0));

    st->Execute();
    tr->Commit();

    return true;
}

bool DataSourceRepository::update(long long id, const DataSourceData& ds)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "UPDATE DATASOURCE "
        "SET NAME = ?, DISKSIZE = ?, SECTORSIZE = ?, HASH = ?, FORMAT = ?, "
        "FILEPATH = ?, TIMEZONE = ?, NOTES = ?, INDEXED = ? "
        "WHERE DATASOURCEID = ?"
        );

    st->Set(1, ds.name.toStdString());
    st->Set(2, static_cast<int64_t>(ds.diskSize));
    st->Set(3, static_cast<int32_t>(ds.sectorSize));
    st->Set(4, ds.hash.toStdString());
    st->Set(5, ds.format.toStdString());
    st->Set(6, ds.filePath.toStdString());
    st->Set(7, ds.timeZone.toStdString());
    st->Set(8, ds.notes.toStdString());
    st->Set(9, static_cast<int16_t>(ds.indexed ? 1 : 0));
    st->Set(10, static_cast<int64_t>(id));

    st->Execute();
    tr->Commit();

    return true;
}

bool DataSourceRepository::remove(long long id)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare("DELETE FROM DATASOURCE WHERE DATASOURCEID = ?");
    st->Set(1, static_cast<int64_t>(id));

    st->Execute();
    tr->Commit();

    return true;
}
