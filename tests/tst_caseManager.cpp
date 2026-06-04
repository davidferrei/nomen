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

QTEST_MAIN(TEST_CaseManager)
#include "tst_caseManager.moc"