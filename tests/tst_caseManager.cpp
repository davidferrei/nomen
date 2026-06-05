#include <QtTest>
#include <QSettings>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/caseManager/casemanager.h"
#include "data/case/caseInformationData.h"

class TEST_CaseManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    // Tests
    void TEST_CreateCaseInfo();
    void TEST_LoadCaseInfo();
    void TEST_OpenCase();
    void TEST_ValidateCaseSchema();
    void TEST_InsertAndValidateInvestigator();
    void TEST_InsertAndValidateOrganization();
    void TEST_InsertAndValidateDataSource();

private:
    QTemporaryDir tst_CaseDir;
    CaseManager* tst_CaseManagerInstance = nullptr;
    CaseInfo tst_CaseInfo;
    
    
    void validateCaseInfoJson(const QString& casePath, const CaseInfo& expectedInfo)
    {
        QFile caseFile(casePath + "/case_info.json");
        QVERIFY2(caseFile.exists(), "case_info.json Not Found");
        
        QVERIFY2(caseFile.open(QIODevice::ReadOnly), "Cannot Open case_info.json");
        QJsonDocument jsonCaseFile = QJsonDocument::fromJson(caseFile.readAll());
        caseFile.close();
        
        QVERIFY2(jsonCaseFile.isObject(), "case_info.json is Not a Valid JSON Object");
        
        QCOMPARE(jsonCaseFile.object().value("case_id").toString(), expectedInfo.caseId);
        QCOMPARE(jsonCaseFile.object().value("case_name").toString(), expectedInfo.caseName);
        QCOMPARE(jsonCaseFile.object().value("notes").toString(), expectedInfo.notes);
    }
};

//###############################################################################################

void TEST_CaseManager::initTestCase()
{
    setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
    setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
    setenv("FIREBIRD_LOCK", "/tmp/firebird_lock", 1);
    
    qDebug() << "Test Suite: TEST_CaseManager Started: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << "Test Case Path:" << tst_CaseDir.path();

    tst_CaseManagerInstance = new CaseManager();
    QVERIFY(tst_CaseDir.isValid());
}

void TEST_CaseManager::cleanupTestCase()
{
    qDebug() <<  "Test Suite: TEST_CaseManager Finished: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void TEST_CaseManager::init()
{
    qDebug() << "Test Started: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void TEST_CaseManager::cleanup()
{
    qDebug() << "Test Finished: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

//###############################################################################################

void TEST_CaseManager::TEST_CreateCaseInfo()
{
    tst_CaseInfo.caseId = "TEST_CASE_001";
    tst_CaseInfo.caseName = "Test Case";
    tst_CaseInfo.createdDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    tst_CaseInfo.updatedDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    tst_CaseInfo.notes = "This is a test case";
    tst_CaseInfo.caseState = 0;
    tst_CaseInfo.casePath = tst_CaseDir.path() + "/test_case";
    
    
    QVERIFY(tst_CaseManagerInstance->createCaseInfo(tst_CaseInfo));
    
    QDir tst_caseDir(tst_CaseInfo.casePath);
    QVERIFY(tst_caseDir.exists("indexes"));
    QVERIFY(tst_caseDir.exists("case_db"));
    QVERIFY(tst_caseDir.exists("evidence"));
    QVERIFY(tst_caseDir.exists("logs"));
    QVERIFY(tst_caseDir.exists("reports"));
    QVERIFY(tst_caseDir.exists("exports"));
    
    QFile caseFile(tst_CaseInfo.casePath + "/case_info.json");
    QVERIFY(caseFile.exists());
    
    validateCaseInfoJson(tst_CaseInfo.casePath, tst_CaseInfo);
}

void TEST_CaseManager::TEST_LoadCaseInfo()
{
    QVERIFY(!tst_CaseInfo.casePath.isEmpty());
    QVERIFY(tst_CaseManagerInstance->loadCaseInfo(tst_CaseInfo.casePath));
    
    CaseInfo& loadedInfo = tst_CaseManagerInstance->getCurrentCase();
    QCOMPARE(loadedInfo.caseId, tst_CaseInfo.caseId);
    QCOMPARE(loadedInfo.caseName, tst_CaseInfo.caseName);
    QCOMPARE(loadedInfo.notes, tst_CaseInfo.notes);
    QCOMPARE(loadedInfo.caseState, tst_CaseInfo.caseState);
    QCOMPARE(loadedInfo.casePath, tst_CaseInfo.casePath);

}


void TEST_CaseManager::TEST_OpenCase()
{
    QVERIFY(!tst_CaseInfo.casePath.isEmpty());
    QVERIFY(!tst_CaseManagerInstance->getCurrentCase().isEnvInitialized);
    QVERIFY(tst_CaseManagerInstance->openCase(tst_CaseInfo.casePath));
    
    CaseInfo& openedCase = tst_CaseManagerInstance->getCurrentCase();
    QCOMPARE(openedCase.caseId, tst_CaseInfo.caseId);
    QCOMPARE(openedCase.caseName, tst_CaseInfo.caseName);
    QCOMPARE(openedCase.notes, tst_CaseInfo.notes);
    QCOMPARE(openedCase.casePath, tst_CaseInfo.casePath);
       
    validateCaseInfoJson(tst_CaseInfo.casePath, tst_CaseInfo);
}

void TEST_CaseManager::TEST_ValidateCaseSchema()
{
    QVERIFY(!tst_CaseInfo.casePath.isEmpty());

    IBPP::Database tst_db = tst_CaseManagerInstance->getCaseDatabaseHandle();
    QVERIFY2(tst_db != nullptr, "Database Handle Not Available from CaseManager");

    try 
    {
        IBPP::Transaction tr = IBPP::TransactionFactory(tst_db);
        tr->Start();

        IBPP::Statement st = IBPP::StatementFactory(tst_db, tr);

        st->Prepare(
            "SELECT RDB$RELATION_NAME FROM RDB$RELATIONS "
            "WHERE RDB$RELATION_NAME IN ('CASETABLE', 'ORGANIZATION', 'INVESTIGATOR', 'DATASOURCE', 'INGESTJOBSHISTORY', 'INGESTFILETABLE', 'INGESTRESULTITEMS')"
        );
        st->Execute();

        int tableCount = 0;
        while (st->Fetch()) 
        {
            std::string tableName;
            st->Get(1, tableName);
            QString trimmedName = QString::fromStdString(tableName).trimmed();
            qDebug() << "Found Table:" << trimmedName;
            tableCount++;
        }

        tr->Commit();

        QVERIFY2(tableCount == 7, QString("Expected 7 Tables, found %1").arg(tableCount).toStdString().c_str());
        qDebug() << "Case Database Schema Validated Successfully: All 7 Tables Exist";
    }
    catch (const IBPP::Exception &e) 
    {
        QFAIL(QString("Failed to Validate Case Database Schema: %1").arg(e.ErrorMessage()).toStdString().c_str());
    }
}

void TEST_CaseManager::TEST_InsertAndValidateInvestigator()
{
    InvestigatorData tst_InvestigadorData;
    tst_InvestigadorData.name = "John Forensics";
    tst_InvestigadorData.phoneNumber = "911111111";
    tst_InvestigadorData.email = "john.forensics@email.com";
    tst_InvestigadorData.notes = "Lead Investigator - Case";

    QVERIFY2(tst_CaseManagerInstance->saveInvestigator(tst_InvestigadorData), "Failed to Save Investigator");

    IBPP::Database tst_db = tst_CaseManagerInstance->getCaseDatabaseHandle();
    QVERIFY2(tst_db != nullptr, "Database Handle Not Available from CaseManager");

    try 
    {
        IBPP::Transaction tr = IBPP::TransactionFactory(tst_db);
        tr->Start();

        IBPP::Statement st = IBPP::StatementFactory(tst_db, tr);
        st->Prepare(
            "SELECT NAME, PHONENUMBER, EMAIL, NOTES FROM INVESTIGATOR "
            "WHERE NAME = ?"
        );
        st->Set(1, tst_InvestigadorData.name.toStdString());
        st->Execute();

        QVERIFY2(st->Fetch(), "Investigator Not Found in Case Database");

        std::string name, phone, email, notes;
        st->Get(1, name);
        st->Get(2, phone);
        st->Get(3, email);
        st->Get(4, notes);

        tr->Commit();

        QCOMPARE(QString::fromStdString(name).trimmed(), tst_InvestigadorData.name);
        QCOMPARE(QString::fromStdString(phone).trimmed(), tst_InvestigadorData.phoneNumber);
        QCOMPARE(QString::fromStdString(email).trimmed(), tst_InvestigadorData.email);
        QCOMPARE(QString::fromStdString(notes).trimmed(), tst_InvestigadorData.notes);

        qDebug() << "Investigator Successfully Saved and Validated in Case Database";
    }
    catch (const IBPP::Exception &e) 
    {
        QFAIL(QString("Failed to Validate Investigator: %1").arg(e.ErrorMessage()).toStdString().c_str());
    }
}

void TEST_CaseManager::TEST_InsertAndValidateOrganization()
{

    OrganizationData tst_OrganizationData;
    tst_OrganizationData.name = "The Organization";
    tst_OrganizationData.phoneNumber = "912121212";
    tst_OrganizationData.email = "the_org@orgs.com";
    tst_OrganizationData.notes = "Organization Notes";

    QVERIFY2(tst_CaseManagerInstance->saveOrganization(tst_OrganizationData), "Failed to Save Organization");

    IBPP::Database tst_db = tst_CaseManagerInstance->getCaseDatabaseHandle();
    QVERIFY2(tst_db != nullptr, "Database Handle Not Available from CaseManager");

    try 
    {
        IBPP::Transaction tr = IBPP::TransactionFactory(tst_db);
        tr->Start();

        IBPP::Statement st = IBPP::StatementFactory(tst_db, tr);
        st->Prepare(
            "SELECT NAME, PHONENUMBER, EMAIL, NOTES FROM ORGANIZATION "
            "WHERE NAME = ?"
        );
        st->Set(1, tst_OrganizationData.name.toStdString());
        st->Execute();

        QVERIFY2(st->Fetch(), "Organization Not Found in Case Database");

        std::string name, phone, email, notes;
        st->Get(1, name);
        st->Get(2, phone);
        st->Get(3, email);
        st->Get(4, notes);

        tr->Commit();

        QCOMPARE(QString::fromStdString(name).trimmed(), tst_OrganizationData.name);
        QCOMPARE(QString::fromStdString(phone).trimmed(), tst_OrganizationData.phoneNumber);
        QCOMPARE(QString::fromStdString(email).trimmed(), tst_OrganizationData.email);
        QCOMPARE(QString::fromStdString(notes).trimmed(), tst_OrganizationData.notes);

        qDebug() << "Organization Successfully Saved and Validated in Case Database";
    }
    catch (const IBPP::Exception &e) 
    {
        QFAIL(QString("Failed to Validate Organization: %1").arg(e.ErrorMessage()).toStdString().c_str());
    }
}

void TEST_CaseManager::TEST_InsertAndValidateDataSource()
{

    DataSourceData tst_DataSourceData;
    tst_DataSourceData.name = "Evidence Disk - 1";
    tst_DataSourceData.diskSize = 2199023255552LL; 
    tst_DataSourceData.sectorSize = 4096;
    tst_DataSourceData.hash = "11111111111111111111111111111111";
    tst_DataSourceData.hashType = "SHA256";
    tst_DataSourceData.format = "RAW";
    tst_DataSourceData.filePath = tst_CaseInfo.casePath + "/evidence/disk.dd";
    tst_DataSourceData.timeZone = "WEST";
    tst_DataSourceData.notes = "Primary Evidence Disk for the Case";
    tst_DataSourceData.indexed = 0;

    QVERIFY2(tst_CaseManagerInstance->saveDataSource(tst_DataSourceData), "Failed to Save DataSource");

    IBPP::Database tst_db = tst_CaseManagerInstance->getCaseDatabaseHandle();
    QVERIFY2(tst_db != nullptr, "Database Handle Not Available from CaseManager");

    try 
    {
        IBPP::Transaction tr = IBPP::TransactionFactory(tst_db);
        tr->Start();

        IBPP::Statement st = IBPP::StatementFactory(tst_db, tr);
        st->Prepare(
            "SELECT NAME, DISKSIZE, SECTORSIZE, HASH_TYPE, FORMAT, FILEPATH, TIMEZONE FROM DATASOURCE "
            "WHERE NAME = ?"
        );
        st->Set(1, tst_DataSourceData.name.toStdString());
        st->Execute();

        QVERIFY2(st->Fetch(), "DataSource Not Found in Case Database");

        std::string name, hashType, format, filePath, timeZone;
        long diskSize;
        int sectorSize;

        st->Get(1, name);
        st->Get(2, diskSize);
        st->Get(3, sectorSize);
        st->Get(4, hashType);
        st->Get(5, format);
        st->Get(6, filePath);
        st->Get(7, timeZone);

        tr->Commit();

        QCOMPARE(QString::fromStdString(name).trimmed(), tst_DataSourceData.name);
        // QCOMPARE(diskSize, tst_DataSourceData.diskSize); //Not Implemented
        // QCOMPARE(sectorSize, tst_DataSourceData.sectorSize); //Not Implemented
        QCOMPARE(QString::fromStdString(hashType).trimmed(), tst_DataSourceData.hashType);
        QCOMPARE(QString::fromStdString(format).trimmed(), tst_DataSourceData.format);
        QCOMPARE(QString::fromStdString(timeZone).trimmed(), tst_DataSourceData.timeZone);

        qDebug() << "DataSource Successfully Saved and Validated in Case Database";
    }
    catch (const IBPP::Exception &e) 
    {
        QFAIL(QString("Failed to Validate DataSource: %1").arg(e.ErrorMessage()).toStdString().c_str());
    }
}



QTEST_MAIN(TEST_CaseManager)
#include "tst_caseManager.moc"
