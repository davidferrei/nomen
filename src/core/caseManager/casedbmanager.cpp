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
                CaseID VARCHAR(64) PRIMARY KEY,
                CaseName VARCHAR(255),
                FilesSavePath VARCHAR(1024),
                Notes BLOB SUB_TYPE TEXT,
                State BOOLEAN DEFAULT TRUE,
                CreationDate TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                LastModifiedDate TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )");

        // EXAMINER Table
        st->Execute(R"(
            CREATE TABLE Examiner (
                ExaminerID VARCHAR(64) PRIMARY KEY,
                Name VARCHAR(255),
                PhoneNumber VARCHAR(50),
                Email VARCHAR(255),
                Notes BLOB SUB_TYPE TEXT
            )
        )");

        // Investigator Table
        st->Execute(R"(
            CREATE TABLE Investigator (
                InvestigatorID VARCHAR(64) PRIMARY KEY,
                Name VARCHAR(255),
                PhoneNumber VARCHAR(50),
                Email VARCHAR(255),
                Notes BLOB SUB_TYPE TEXT
            )
        )");

        tr->Commit();
        db->Disconnect();

        qDebug() << "FIREBIRD: Schema Created Successfully";
        return true;
    }
    catch (const IBPP::Exception &e)
    {
        qCritical() << "FIREBIRD: Failed to create schema:" << e.ErrorMessage();
        return false;
    }
}
