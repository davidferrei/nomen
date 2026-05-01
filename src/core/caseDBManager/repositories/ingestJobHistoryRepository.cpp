#include "ingestJobHistoryRepository.h"

IngestJobHistoryRepository::IngestJobHistoryRepository(IBPP::Database db) : fb_db(db) {}

bool IngestJobHistoryRepository::insert(IngestJobHistoryData &job)
{
    qDebug() << "[IngestJobHistoryRepository] Inserting IngestJobHistoryData: Module=" << job.moduleName << " Status=" << job.status;

    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "INSERT INTO IngestJobsHistory"
        " (MODULE_NAME, MODULE_VERSION, STATUS, LOG, SETTINGS_JSON)"
        " VALUES (?, ?, ?, ?, ?)"
    );

    st->Set(1, job.moduleName.isEmpty() ? "" : job.moduleName.toStdString());
    st->Set(2, job.moduleVersion.isEmpty() ? "" : job.moduleVersion.toStdString());
    st->Set(3, static_cast<int16_t>(job.status));
    st->Set(4, job.log.isEmpty() ? "" : job.log.toStdString());
    st->Set(5, job.settingsJson.isEmpty() ? "" : job.settingsJson.toStdString());

    st->Execute();

    st->Prepare("SELECT GEN_ID(GEN_INGESTJOB_ID, 0) FROM RDB$DATABASE");
    st->Execute();
    if (st->Fetch()) 
    {
        int64_t newid = 0;
        st->Get(1, newid);
        job.id = static_cast<qint64>(newid);
    }

    tr->Commit();

    return true;
}

bool IngestJobHistoryRepository::update(qint64 id, const IngestJobHistoryData &job)
{
    qDebug() << "[IngestJobHistoryRepository] Updating IngestJobHistoryData: ID=" << id << " Status=" << job.status;

    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "UPDATE IngestJobsHistory SET END_TIME = CURRENT_TIMESTAMP, STATUS = ?, LOG = ? WHERE ID = ?"
    );

    st->Set(1, static_cast<int16_t>(job.status));
    st->Set(2, job.log.isEmpty() ? "" : job.log.toStdString());
    st->Set(3, static_cast<int64_t>(id));

    st->Execute();
    tr->Commit();
    return true;

}

bool IngestJobHistoryRepository::remove(qint64 id)
{
    qDebug() << "[IngestJobHistoryRepository] Removing IngestJobHistoryData: ID=" << id;

    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare("DELETE FROM IngestJobsHistory WHERE ID = ?");
    st->Set(1, static_cast<int64_t>(id));

    st->Execute();
    tr->Commit();

    return true;

}


//############################################################################################
// Getters

QList<IngestJobHistoryData> IngestJobHistoryRepository::getAll()
{
    qDebug() << "[IngestJobHistoryRepository] Retrieving All IngestJobHistoryData";
    QList<IngestJobHistoryData> result;

    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "SELECT ID, MODULE_NAME, MODULE_VERSION,"
        " CAST(EXTRACT(YEAR   FROM START_TIME) AS INTEGER),"
        " CAST(EXTRACT(MONTH  FROM START_TIME) AS INTEGER),"
        " CAST(EXTRACT(DAY    FROM START_TIME) AS INTEGER),"
        " CAST(EXTRACT(HOUR   FROM START_TIME) AS INTEGER),"
        " CAST(EXTRACT(MINUTE FROM START_TIME) AS INTEGER),"
        " CAST(EXTRACT(SECOND FROM START_TIME) AS INTEGER),"
        " CAST(EXTRACT(YEAR   FROM END_TIME)   AS INTEGER),"
        " CAST(EXTRACT(MONTH  FROM END_TIME)   AS INTEGER),"
        " CAST(EXTRACT(DAY    FROM END_TIME)   AS INTEGER),"
        " CAST(EXTRACT(HOUR   FROM END_TIME)   AS INTEGER),"
        " CAST(EXTRACT(MINUTE FROM END_TIME)   AS INTEGER),"
        " CAST(EXTRACT(SECOND FROM END_TIME)   AS INTEGER),"
        " STATUS, LOG, SETTINGS_JSON"
        " FROM IngestJobsHistory"
    );
    st->Execute();

    while (st->Fetch()) 
    {
        IngestJobHistoryData job;

        int64_t idtmp = 0;
        st->Get(1, idtmp);
        job.id = static_cast<qint64>(idtmp);

        std::string s;
        if (st->Get(2, s)) job.moduleName = QString::fromStdString(s);
        s.clear();
        if (st->Get(3, s)) job.moduleVersion = QString::fromStdString(s);

        if (!st->IsNull(4)) 
        {
            int32_t sy = 0, smo = 0, sd = 0, shh = 0, smi = 0, sss = 0;
            st->Get(4, sy); st->Get(5, smo); st->Get(6, sd);
            st->Get(7, shh); st->Get(8, smi); st->Get(9, sss);
            QDateTime dt(QDate(sy, smo, sd), QTime(shh, smi, sss));
            job.startTime = dt.isValid() ? dt : QDateTime();
        }

        if (!st->IsNull(10)) 
        {
            int32_t ey = 0, emo = 0, ed = 0, ehh = 0, emi = 0, ess = 0;
            st->Get(10, ey); st->Get(11, emo); st->Get(12, ed);
            st->Get(13, ehh); st->Get(14, emi); st->Get(15, ess);
            QDateTime dt(QDate(ey, emo, ed), QTime(ehh, emi, ess));
            job.endTime = dt.isValid() ? dt : QDateTime();
        }

        int16_t stat = 0;
        st->Get(16, stat);
        job.status = stat;

        s.clear();
        if (st->Get(17, s)) job.log = QString::fromStdString(s);
        
        s.clear();
        if (st->Get(18, s)) job.settingsJson = QString::fromStdString(s);

        result.append(job);
    }

    tr->Commit();


    return result;
}


//############################################################################################
// Setters



bool IngestJobHistoryRepository::appendLog(qint64 id, const QString &logLine)
{
    qDebug() << "[IngestJobHistoryRepository] Appending log to IngestJobHistoryData: ID=" << id << " Log Length=" << logLine.size();
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    try 
    {
        IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
        st->Prepare(
            "UPDATE IngestJobsHistory "
            "SET LOG = CASE WHEN COALESCE(LOG,'') = '' THEN ? ELSE LOG || ? END "
            "WHERE ID = ?"
        );

        std::string frag1 = logLine.isEmpty() ? std::string("") : logLine.toStdString();
        std::string frag2 = logLine.isEmpty() ? std::string("") : (std::string("\n") + logLine.toStdString());

        st->Set(1, frag1);
        st->Set(2, frag2);
        st->Set(3, static_cast<int64_t>(id));

        st->Execute();
        tr->Commit();
        
        return true;
    }
    catch (const IBPP::Exception &e) 
    {
        qWarning() << "[IngestJobHistoryRepository]::appendLog IBPP Exception:" << e.ErrorMessage();
        try { tr->Rollback(); } catch(...) {}
        return false;
    }
    catch (const std::exception &e) 
    {
        qWarning() << "[IngestJobHistoryRepository]::appendLog Exception:" << e.what();
        try { tr->Rollback(); } catch(...) {}
        return false;
    }
}

bool IngestJobHistoryRepository::setStatus(qint64 id, int status)
{
    qDebug() << "[IngestJobHistoryRepository] Setting status for IngestJobHistoryData: ID=" << id << " Status=" << status;
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare("UPDATE IngestJobsHistory SET END_TIME = CURRENT_TIMESTAMP, STATUS = ? WHERE ID = ?");
    st->Set(1, static_cast<int16_t>(status));
    st->Set(2, static_cast<int64_t>(id));

    st->Execute();
    tr->Commit();

    return true;

}