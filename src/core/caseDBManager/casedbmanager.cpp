#include "casedbmanager.h"


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

        // Case Table
        st->Execute(R"(
            CREATE TABLE CaseTable (
                CASEID VARCHAR(64) PRIMARY KEY,
                NAME VARCHAR(255),
                FILESAVEPATH VARCHAR(1024),
                NOTES BLOB SUB_TYPE TEXT,
                STATE SMALLINT DEFAULT 0,
                CREATIONDATE TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                MODIFIEDDATE TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )");

        // Organization Table
        st->Execute(R"(
            CREATE TABLE Organization (
                ORGANIZATIONID BIGINT PRIMARY KEY,
                NAME VARCHAR(255),
                PHONENUMBER VARCHAR(50),
                EMAIL VARCHAR(255),
                NOTES BLOB SUB_TYPE TEXT
            )
        )");

        // Investigator Table
        st->Execute(R"(
            CREATE TABLE Investigator (
                INVESTIGATORID BIGINT PRIMARY KEY,
                NAME VARCHAR(255),
                PHONENUMBER VARCHAR(50),
                EMAIL VARCHAR(255),
                NOTES BLOB SUB_TYPE TEXT
            )
        )");

        // DataSource Table
        st->Execute(R"(
            CREATE TABLE DataSource (
                DATASOURCEID BIGINT PRIMARY KEY,
                NAME VARCHAR(255),
                DISKSIZE BIGINT,
                SECTORSIZE INTEGER,
                HASH VARCHAR(1024),
                HASH_TYPE VARCHAR(32),
                FORMAT VARCHAR(64),
                FILEPATH VARCHAR(1024),
                TIMEZONE VARCHAR(128),
                NOTES BLOB SUB_TYPE TEXT,
                INDEXED SMALLINT DEFAULT 0
            )
        )");

        // IngestJobsHistory Table
        st->Execute(R"(
            CREATE TABLE IngestJobsHistory (
                ID BIGINT PRIMARY KEY,
                MODULE_NAME VARCHAR(255),
                MODULE_VERSION VARCHAR(64),
                START_TIME TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                END_TIME TIMESTAMP,
                STATUS SMALLINT DEFAULT 0,
                LOG BLOB SUB_TYPE TEXT,
                SETTINGS_JSON BLOB SUB_TYPE TEXT
            )
        )");

        st->Execute(R"(
            CREATE TABLE IngestResultItems (
                ID BIGINT PRIMARY KEY,
                JOB_ID BIGINT NOT NULL REFERENCES IngestJobsHistory(ID),
                PARENT_ITEM_ID BIGINT,
                ITEM_TYPE SMALLINT DEFAULT 0 NOT NULL,
                ITEM_NAME VARCHAR(1024) NOT NULL,
                DATASOURCE_ID BIGINT,
                PARTITION_ID SMALLINT,
                FILESYSTEM_ID SMALLINT,
                LMDB_PATH VARCHAR(4096)
            )
        )");

        // Increment IDs
        // st->Execute("CREATE GENERATOR GEN_CASE_ID;");
        // st->Execute("SET GENERATOR GEN_CASE_ID TO 0;");

        st->Execute("CREATE GENERATOR GEN_INVESTIGATOR_ID");
        st->Execute("SET GENERATOR GEN_INVESTIGATOR_ID TO 0");

        st->Execute("CREATE GENERATOR GEN_ORGANIZATION_ID");
        st->Execute("SET GENERATOR GEN_ORGANIZATION_ID TO 0");

        st->Execute("CREATE GENERATOR GEN_DATASOURCE_ID");
        st->Execute("SET GENERATOR GEN_DATASOURCE_ID TO 0");

        st->Execute("CREATE GENERATOR GEN_INGESTJOB_ID");
        st->Execute("SET GENERATOR GEN_INGESTJOB_ID TO 0");

        st->Execute("CREATE GENERATOR GEN_INGESTRESULT_ID");
        st->Execute("SET GENERATOR GEN_INGESTRESULT_ID TO 0");

        // Triggers
        // st->Execute(R"(
        //     CREATE TRIGGER BI_CASE FOR CASETABLE
        //     BEFORE INSERT
        //     AS
        //     BEGIN
        //         IF (NEW.CASEID IS NULL) THEN
        //             NEW.CASEID = GEN_ID(GEN_CASE_ID, 1);
        //     END
        // )");

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

        st->Execute(R"(
            CREATE TRIGGER BI_DATASOURCE FOR DATASOURCE
            BEFORE INSERT
            AS
            BEGIN
                IF (NEW.DATASOURCEID IS NULL) THEN
                    NEW.DATASOURCEID = GEN_ID(GEN_DATASOURCE_ID, 1);
            END
        )");

        st->Execute(R"(
            CREATE TRIGGER BI_INGESTJOBS FOR INGESTJOBSHISTORY
            BEFORE INSERT
            AS
            BEGIN
                IF (NEW.ID IS NULL) THEN
                    NEW.ID = GEN_ID(GEN_INGESTJOB_ID, 1);
            END
        )");

        st->Execute(R"(
            CREATE TRIGGER BI_INGESTRESULTITEMS FOR INGESTRESULTITEMS
            BEFORE INSERT
            AS
            BEGIN
                IF (NEW.ID IS NULL) THEN
                    NEW.ID = GEN_ID(GEN_INGESTRESULT_ID, 1);
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
    if (fb_db != nullptr && fb_db->Connected()) {
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

IBPP::Database CaseDBManager::database() const
{
    return fb_db;
}
