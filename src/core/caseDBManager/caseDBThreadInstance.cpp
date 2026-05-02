#include "caseDBThreadInstance.h"

CaseDBThreadInstance::CaseDBThreadInstance(const QString& dbFilePath) : dbPath(dbFilePath) {}

CaseDBThreadInstance::~CaseDBThreadInstance()
{
    closeDatabase();
}

bool CaseDBThreadInstance::openDatabase()
{
    qDebug() << "[FIREBIRD]: Opening Case Database";

    const QString dbFilePath = dbPath + "/case_db/case.fdb";

    if (!QFile::exists(dbFilePath)) 
    {
        qCritical() << "[FIREBIRD]: Database File Does not Exist:" << dbFilePath;
        return false;
    }

    try {
        // std::string lockStd = QString("/tmp/firebird_lock_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId())).toStdString();


        // setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
        // setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
        // setenv("FIREBIRD_LOCK", lockStd.c_str(), 1);

        const std::string dbPath = dbFilePath.toStdString();

        fb_db = IBPP::DatabaseFactory(
            "",
            dbPath,
            "SYSDBA", //TODO: Make User Choose This
            "masterkey"
            );

        fb_db->Connect();
        qInfo() << "[FIREBIRD]: Database Connection Successful";

        IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
        tr->Start();

        IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
        st->Prepare(
            "SELECT "
            "CURRENT_USER, "
            "CURRENT_ROLE, "
            "CAST(CURRENT_TIMESTAMP AS VARCHAR(64))"
            "FROM RDB$DATABASE"
            );

        st->Execute();

        std::string user;
        std::string role;
        std::string firebirdTime;

        if (st->Fetch())
        {
            st->Get(1, user);
            st->Get(2, role);
            st->Get(3, firebirdTime);

            qDebug() << "##########################################";
            qDebug() << "[FIREBIRD]: Stats";
            qDebug() << "[FIREBIRD]: Connected User:" << user.c_str();
            qDebug() << "[FIREBIRD]: Role:" << role.c_str();
            qDebug() << "[FIREBIRD]: Server Time:" << firebirdTime.c_str();
            qDebug() << "##########################################";
        }

        tr->Commit();
        //fb_db->Disconnect();

    }
    catch (const IBPP::Exception& e) {
        qCritical() << "[FIREBIRD]: Error While Opening Database:" << e.ErrorMessage();
        return false;
    }
    catch (const std::exception& e) {
        qCritical() << "[FIREBIRD]: Exception While Opening Database:" << e.what();
        return false;
    }
    catch (...) {
        qCritical() << "[FIREBIRD]: Unknown Error While Opening Database";
        return false;
    }

    return true;
}


IBPP::Database CaseDBThreadInstance::database() const
{
    return fb_db;
}


void CaseDBThreadInstance::closeDatabase()
{
    if (fb_db != nullptr && fb_db->Connected())
    {
        qDebug() << "[FIREBIRD]: Closing Database Connection";
        try {
            fb_db->Disconnect();
        }
        catch (...) {
            qWarning() << "[FIREBIRD]: Exception While Closing Case DB";
        }
    }

    fb_db = nullptr;
}

bool CaseDBThreadInstance::databaseExists() const
{
    return QFile::exists(dbPath + "/case_db/case.fdb");
}

