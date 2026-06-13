#include "datasourceRepository.h"

DataSourceRepository::DataSourceRepository(IBPP::Database db) : fb_db(db) {}

//############################################################################################
//DB Operations

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
    qDebug() << "[IndexWorker] Updating DS:" << ds.id;

    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    try
    {
        tr->Start();

        IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
        st->Prepare(
            "UPDATE DATASOURCE "
            "SET NAME = ?, DISKSIZE = ?, SECTORSIZE = ?, HASH = ?, HASH_TYPE = ?, FORMAT = ?, "
            "FILEPATH = ?, TIMEZONE = ?, NOTES = ?, INDEXED = ? "
            "WHERE DATASOURCEID = ?"
            );

        qDebug() << "[IndexWorker] Updated DS - id:" << id
                 << "name:" << ds.name << "diskSize:" << ds.diskSize
                 << "sectorSize:" << ds.sectorSize << "hash:" << ds.hash
                 << "hashType:" << ds.hashType << "format:" << ds.format
                 << "filePath:" << ds.filePath
                 << "timeZone:" << ds.timeZone << "indexed:" << ds.indexed;

        st->Set(1, ds.name.toStdString());
        st->Set(2, static_cast<int64_t>(ds.diskSize));
        st->Set(3, static_cast<int32_t>(ds.sectorSize));
        st->Set(4, ds.hash.toStdString());
        st->Set(5, ds.hashType.toStdString());
        st->Set(6, ds.format.toStdString());
        st->Set(7, ds.filePath.toStdString());
        st->Set(8, ds.timeZone.toStdString());
        st->Set(9, ds.notes.toStdString());
        st->Set(10, static_cast<int16_t>(ds.indexed ? 1 : 0));
        st->Set(11, static_cast<int64_t>(id));

        st->Execute();
        tr->Commit();

        qDebug() << "[IndexWorker] Updated DS:" << ds.id;
        return true;
    }
    catch (const IBPP::SQLException& ex)
    {
        qWarning() << "[IndexWorker] SQLException Updating DS id:" << id << "Error:" << ex.what();
        try { tr->Rollback(); } catch(...) {}
        return false;
    }
    catch (const std::exception& ex)
    {
        qWarning() << "[IndexWorker] std::exception Updating DS id:" << id << "Error:" << ex.what();
        try { tr->Rollback(); } catch(...) {}
        return false;
    }
    catch (...)
    {
        qWarning() << "[IndexWorker] Unknown Exception Updating DS id:" << id;
        try { tr->Rollback(); } catch(...) {}
        return false;
    }
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

//############################################################################################
// Getters


QList<DataSourceData> DataSourceRepository::GetAll()
{
    QList<DataSourceData> result;

    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);

    st->Prepare(
        "SELECT DATASOURCEID, NAME, DISKSIZE, SECTORSIZE, HASH, HASH_TYPE, FORMAT, "
        "FILEPATH, TIMEZONE, NOTES, INDEXED "
        "FROM DATASOURCE"
        );

    st->Execute();

    while (st->Fetch())
    {
        DataSourceData ds;
        std::string tmpString;
        int indexedValue = 0;

        int64_t idTmp;
        st->Get(1, idTmp);
        ds.id = static_cast<qint64>(idTmp);

        st->Get(2, tmpString);
        ds.name = QString::fromUtf8(tmpString.data(), tmpString.size());

        int64_t diskSizeTmp;
        st->Get(3, diskSizeTmp);
        ds.diskSize = static_cast<quint64>(diskSizeTmp);

        int32_t sectorSizeTmp;
        st->Get(4, sectorSizeTmp);
        ds.sectorSize = static_cast<quint32>(sectorSizeTmp);

        st->Get(5, tmpString);
        ds.hash = QString::fromUtf8(tmpString.data(), tmpString.size());

        st->Get(6, tmpString);
        ds.hashType = QString::fromUtf8(tmpString.data(), tmpString.size());

        st->Get(7, tmpString);
        ds.format = QString::fromUtf8(tmpString.data(), tmpString.size());

        st->Get(8, tmpString);
        ds.filePath = QString::fromUtf8(tmpString.data(), tmpString.size());

        st->Get(9, tmpString);
        ds.timeZone = QString::fromUtf8(tmpString.data(), tmpString.size());

        st->Get(10, tmpString);
        ds.notes = QString::fromUtf8(tmpString.data(), tmpString.size());

        st->Get(11, indexedValue);
        ds.indexed = (indexedValue == 1);

        result.append(ds);
    }

    tr->Commit();

    return result;
}
