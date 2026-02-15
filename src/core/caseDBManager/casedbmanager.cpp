#include "casedbmanager.h"

#include <QDebug>

CaseDBManager::CaseDBManager() {}

bool CaseDBManager::createSchema(IBPP::Database db)
{
    try
    {
        db->Connect();
        IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite);

        tr->Start();

        IBPP::Statement st = IBPP::StatementFactory(db, tr);

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
        db->Disconnect();

        qDebug() << "FIREBIRD: Schema Created Successfully";
        return true;
    }
    catch (const IBPP::Exception &e)
    {
        qCritical() << "FIREBIRD: Failed to create schema:" << e.ErrorMessage();
        try { db->Disconnect(); } catch (...) {}
        return false;
    }
}
