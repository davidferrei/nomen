#include "casedbmanager.h"

#include <QDebug>
#include <QDir>

CaseDBManager::CaseDBManager(const QString& casePath) : dbPath(casePath) {}

CaseDBManager::~CaseDBManager()
{
    closeDatabase();
}

bool CaseDBManager::createSchema()
{
    if (fb_db == nullptr) {
        qCritical() << "Database Handle not Initialized!";
        return false;
    }

    try
    {
        fb_db->Connect();
        IBPP::Transaction tr = IBPP::TransactionFactory(fb_db, IBPP::amWrite);

        tr->Start();

        IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);

        // CASE Table
        st->Execute(R"(
            CREATE TABLE CaseTable (
                CASEID VARCHAR(64) PRIMARY KEY,
                NAME VARCHAR(255),
                FILESAVEPATH VARCHAR(1024),
                NOTES BLOB SUB_TYPE TEXT,
                STATE BOOLEAN DEFAULT TRUE,
                CREATIONDATE TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                MODIFIEDDATE TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )");

        // Organization Table
        st->Execute(R"(
            CREATE TABLE Organization (
                ORGANIZATIONID VARCHAR(64) PRIMARY KEY,
                NAME VARCHAR(255),
                PHONENUMBER VARCHAR(50),
                EMAIL VARCHAR(255),
                NOTES BLOB SUB_TYPE TEXT
            )
        )");

        // Investigator Table
        st->Execute(R"(
            CREATE TABLE Investigator (
                INVESTIGATORID VARCHAR(64) PRIMARY KEY,
                NAME VARCHAR(255),
                PHONENUMBER VARCHAR(50),
                EMAIL VARCHAR(255),
                NOTES BLOB SUB_TYPE TEXT
            )
        )");

        // Increment IDs
        st->Execute("CREATE GENERATOR GEN_INVESTIGATOR_ID");
        st->Execute("SET GENERATOR GEN_INVESTIGATOR_ID TO 0");

        st->Execute("CREATE GENERATOR GEN_ORGANIZATION_ID");
        st->Execute("SET GENERATOR GEN_ORGANIZATION_ID TO 0");

        // Triggers
        st->Execute(R"(
            CREATE TRIGGER BI_INVESTIGATORS FOR INVESTIGATOR
            BEFORE INSERT
            AS
            BEGIN
                IF (NEW.INVESTIGATORID IS NULL) THEN
                    NEW.INVESTIGATORID = GEN_ID(GEN_INVESTIGATOR_ID, 1);
            END
        )");

        st->Execute(R"(
            CREATE TRIGGER BI_ORGANIZATIONS FOR ORGANIZATION
            BEFORE INSERT
            AS
            BEGIN
                IF (NEW.ORGANIZATIONID IS NULL) THEN
                    NEW.ORGANIZATIONID = GEN_ID(GEN_ORGANIZATION_ID, 1);
            END
        )");


        tr->Commit();
        fb_db->Disconnect();

        qDebug() << "FIREBIRD: Schema Created Successfully";
        return true;
    }
    catch (const IBPP::Exception &e)
    {
        qCritical() << "FIREBIRD: Failed to create schema:" << e.ErrorMessage();
        try { fb_db->Disconnect(); } catch (...) {}
        return false;
    }
}

//##

bool CaseDBManager::createDatabase()
{
    qDebug() << "FIREBIRD: Creating Case Database";

    if (dbPath.isEmpty()) {
        qCritical() << "No Case Path Set";
        return false;
    }

    const QString dbDirPath = dbPath + "/case_db";
    const QString dbFilePath = dbDirPath + "/case.fdb";

    QDir caseDbDir(dbDirPath);
    if (!caseDbDir.exists()) {
        if (!caseDbDir.mkpath(".")) {
            qCritical() << "Failed to Create case_db Directory:" << dbDirPath;
            return false;
        }
    }


    //TODO: Windows Fix
    setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
    setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
    setenv("FIREBIRD_LOCK", "/tmp/firebird_lock", 1);

    QDir lockDir("/tmp/firebird_lock"); //TODO: Windows Fix

    if (!lockDir.exists()) {
        if (!lockDir.mkpath(".")) {
            qCritical() << "Failed to Create Firebird Lock Directory:" << lockDir.absolutePath();
            return false;
        }
    }

    try {
        const std::string dbPath = dbFilePath.toStdString();

        fb_db = IBPP::DatabaseFactory(
            "",
            dbPath,
            "SYSDBA", //TODO: Make User Choose This
            "masterkey"
            );

        if (!std::filesystem::exists(dbPath)) {
            fb_db->Create(3);
            fb_db->Disconnect();
        } else {
            qInfo() << "FIREBIRD: Database Already exists, Skipping Creation";
        }
    }
    catch (const IBPP::Exception& e) {
        qCritical() << "FIREBIRD: Error While Creating Database:" << e.ErrorMessage();
        return false;
    }
    catch (const std::exception& e) {
        qCritical() << "Exception While Creating Database:" << e.what();
        return false;
    }
    catch (...) {
        qCritical() << "FIREBIRD: Unknown Error While Creating Database";
        return false;
    }

    return true;
}


bool CaseDBManager::openDatabase()
{
    qDebug() << "FIREBIRD: Opening Case Database";

    const QString dbFilePath = dbPath + "/case_db/case.fdb";

    if (!QFile::exists(dbFilePath)) {
        qCritical() << "Database File Does not Exist:" << dbFilePath;
        return false;
    }

    try {
        setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
        setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
        setenv("FIREBIRD_LOCK", "/tmp/firebird_lock", 1);

        const std::string dbPath = dbFilePath.toStdString();

        fb_db = IBPP::DatabaseFactory(
            "",
            dbPath,
            "SYSDBA", //TODO: Make User Choose This
            "masterkey"
            );

        fb_db->Connect();
        qInfo() << "FIREBIRD: Database Connection Successful";

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

        if (st->Fetch()) {
            st->Get(1, user);
            st->Get(2, role);
            st->Get(3, firebirdTime);

            qDebug() << "##########################################";
            qDebug() << "FIREBIRD Stats";
            qDebug() << "FIREBIRD: Connected User:" << user.c_str();
            qDebug() << "FIREBIRD: Role:" << role.c_str();
            qDebug() << "FIREBIRD: Server Time:" << firebirdTime.c_str();
            qDebug() << "##########################################";
        }

        tr->Commit();
        //fb_db->Disconnect();

    }
    catch (const IBPP::Exception& e) {
        qCritical() << "FIREBIRD: Error While Opening Database:" << e.ErrorMessage();
        return false;
    }
    catch (const std::exception& e) {
        qCritical() << "Exception While Opening Database:" << e.what();
        return false;
    }
    catch (...) {
        qCritical() << "FIREBIRD: Unknown Error While Opening Database";
        return false;
    }

    return true;
}


void CaseDBManager::closeDatabase()
{
    if (fb_db->Connected()) {
        qDebug() << "FIREBIRD: Closing Database Connection";
        try {
            fb_db->Disconnect();
        }
        catch (...) {
            qWarning() << "FIREBIRD: Exception While Closing Case DB";
        }
    }

    fb_db = nullptr;
}

bool CaseDBManager::databaseExists() const
{
    return QFile::exists(dbPath + "/case_db/case.fdb");
}
