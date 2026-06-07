
#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDateTime>

#include "core/caseManager/casemanager.h"
#include "core/jobManager/jobManager.h"
#include "core/ingest/repo/ingestResultDBInstance.h"
#include "core/extensions/ingestModuleFactory.h"
#include "data/case/caseInformationData.h"
#include "data/ingest/ingestJobHistoryData.h"

class TEST_IngestDisk : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    // Tests
    void TEST_CreateAndOpenCase();
    void TEST_CreateIngestJobs();
    void TEST_ValidateIngestResults();
    void TEST_ConcurrentTxtCounterJobs();

private:
    QTemporaryDir tst_CaseDir;
    CaseManager* tst_CaseManagerInstance = nullptr;
    JobManager* tst_JobManagerInstance = nullptr;
    CaseInfo tst_CaseInfo;
    DataSourceData tst_DataSource;
    
    const QString TEST_DISK_PATH = "/mnt/disk01/DiskImage/kali.raw";
    const QStringList INGEST_MODULES = {"DetectMIMEType", "TxtCounter"};
};

//###############################################################################################

void TEST_IngestDisk::initTestCase()
{
    setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
    setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
    setenv("FIREBIRD_LOCK", "/tmp/firebird_lock", 1);
    
    qDebug() << "Test Suite: TEST_IngestDisk Started:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << "Test Case Path:" << tst_CaseDir.path();
    qDebug() << "Test Disk:" << TEST_DISK_PATH;

    IngestModuleFactory::instance();
    qDebug() << "Ingest Modules Loaded:" << IngestModuleFactory::instance().moduleNamesList();

    tst_CaseManagerInstance = new CaseManager();
    tst_JobManagerInstance = new JobManager();
    
    QVERIFY(tst_CaseDir.isValid());
    QVERIFY2(QFile::exists(TEST_DISK_PATH), "Test Disk Not Found");
}

void TEST_IngestDisk::cleanupTestCase()
{
    qDebug() << "Test Suite: TEST_IngestDisk Finished:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    
    if (tst_CaseManagerInstance != nullptr) 
    {
        delete tst_CaseManagerInstance;
    }

    if (tst_JobManagerInstance != nullptr) 
    {
        delete tst_JobManagerInstance;
    }
}

void TEST_IngestDisk::init()
{
    qDebug() << "Test Started:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void TEST_IngestDisk::cleanup()
{
    qDebug() << "Test Finished:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

//###############################################################################################

void TEST_IngestDisk::TEST_CreateAndOpenCase()
{
    tst_CaseInfo.caseId = "TEST_INGEST_CASE_001";
    tst_CaseInfo.caseName = "Ingest Test Case";
    tst_CaseInfo.createdDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    tst_CaseInfo.updatedDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    tst_CaseInfo.notes = "Test Case for Disk Ingestion";
    tst_CaseInfo.caseState = 0;
    tst_CaseInfo.casePath = tst_CaseDir.path() + "/ingest_test_case";
    
    QVERIFY2(tst_CaseManagerInstance->createCaseInfo(tst_CaseInfo), "Failed to Create Case Info");
       
    QVERIFY2(tst_CaseManagerInstance->openCase(tst_CaseInfo.casePath), "Failed to Open Case");
    
    CaseInfo& openedCase = tst_CaseManagerInstance->getCurrentCase();
    QCOMPARE(openedCase.caseId, tst_CaseInfo.caseId);
    
    qDebug() << "Case Successfully Created and Opened for Ingestion Tests";
}

void TEST_IngestDisk::TEST_CreateIngestJobs()
{
    // QVERIFY(!tst_CaseInfo.casePath.isEmpty());

    // tst_DataSource.name = "Ingest Disk - 0";
    // tst_DataSource.diskSize = 0;
    // tst_DataSource.sectorSize = 512;
    // tst_DataSource.hash = "";
    // tst_DataSource.hashType = "SHA256";
    // tst_DataSource.format = "RAW";
    // tst_DataSource.filePath = TEST_DISK_PATH;
    // tst_DataSource.timeZone = "UTC";
    // tst_DataSource.notes = "Test Disk for Ingestion";
    // tst_DataSource.indexed = 0;

    // QVERIFY2(tst_CaseManagerInstance->saveDataSource(tst_DataSource), "Failed to Save Data Source");
    
    // QList<DataSourceData> DataSourceList = tst_CaseManagerInstance->getDataSources();
    // for (const DataSourceData& ds : DataSourceList) 
    // {
    //     if (ds.filePath == TEST_DISK_PATH && ds.name == tst_DataSource.name) 
    //     {
    //         tst_DataSource = ds;
    //         break;
    //     }
    // }
    
    // QVERIFY2(tst_DataSource.id > 0, "Data Source not Assigned an ID in Database");
    // QVERIFY2(tst_JobManagerInstance != nullptr, "JobManager not Initialized");
    
    // qDebug() << "Ingest Jobs Created for:" << tst_DataSource.name;
    // qDebug() << "Disk Path:" << tst_DataSource.filePath;
    // qDebug() << "Ingest Modules:" << INGEST_MODULES;

   
    // QSignalSpy ingestFinishedSpy(tst_JobManagerInstance, SIGNAL(jobFinished(QString, Job::JobType, QString, QString, qint64)));
    // QSignalSpy jobFailedSpy(tst_JobManagerInstance, SIGNAL(jobFailed(QString, QString, Job::JobType, QString, QString, qint64)));

    // tst_JobManagerInstance->CreateIngestJob(tst_DataSource, tst_CaseInfo.casePath, "DetectMIMEType");
    // qDebug() << "Started Ingest Job for Module: DetectMIMEType";
    
    // while (ingestFinishedSpy.count() < 1 && jobFailedSpy.count() == 0) 
    // {
    //     ingestFinishedSpy.wait(60000);
    // }
    
    // QVERIFY2(jobFailedSpy.count() == 0, "DetectMIMEType Ingest Job Failed");
    // QVERIFY2(ingestFinishedSpy.count() >= 1, "DetectMIMEType Ingest Job Timeout");
    
    
    // QBENCHMARK 
    // {
    //     QSignalSpy ingestFinishedSpy(tst_JobManagerInstance, SIGNAL(jobFinished(QString, Job::JobType, QString, QString, qint64)));
    //     QSignalSpy jobFailedSpy(tst_JobManagerInstance, SIGNAL(jobFailed(QString, QString, Job::JobType, QString, QString, qint64)));

    //     tst_JobManagerInstance->CreateIngestJob(tst_DataSource, tst_CaseInfo.casePath, "TxtCounter");
    //     qDebug() << "Started Ingest Job for Module: TxtCounter";
        
    //     while (ingestFinishedSpy.count() < 1 && jobFailedSpy.count() == 0) 
    //     {
    //         ingestFinishedSpy.wait(60000);
    //     }
        
    //     QVERIFY2(jobFailedSpy.count() == 0, "TxtCounter Ingest Job Failed");
    //     QVERIFY2(ingestFinishedSpy.count() >= 1, "TxtCounter Ingest Job Timeout");
    // }
    
    // qDebug() << "All Ingest Jobs Completed Successfully";
}


void TEST_IngestDisk::TEST_ValidateIngestResults()
{
    // QVERIFY(!tst_CaseInfo.casePath.isEmpty());
    
    // QList<IngestJobHistoryData> ingestJobs = tst_CaseManagerInstance->getIngestJobs();
    // QVERIFY2(ingestJobs.size() > 0, "No Ingest Jobs Found in Case Database");
    
    // qDebug() << "Total Ingest Jobs Found:" << ingestJobs.size();
    
    // int jobsValidated = 0;
    
    // for (const QString& module : INGEST_MODULES)
    // {
    //     IngestJobHistoryData targetJob;
    //     for (const IngestJobHistoryData& job : ingestJobs) 
    //     {
    //         if (job.moduleName == module) 
    //         {
    //             targetJob = job;
    //             break;
    //         }
    //     }
        
    //     QVERIFY2(targetJob.id > 0, QString("Ingest Job not Found for Module: %1").arg(module).toStdString().c_str());
        
    //     QString jobStatus;
    //     switch (targetJob.status)
    //     {
    //         case 0: 
    //             jobStatus = "Pending"; 
    //             break;
    //         case 1: 
    //             jobStatus = "Running"; 
    //             break;
    //         case 2: 
    //             jobStatus = "OK"; 
    //             break;
    //         case 3: 
    //             jobStatus = "NOK"; 
    //             break;
    //         default: 
    //             jobStatus = "Unknown"; 
    //             break;
    //     }
        
    //     qDebug() << "Found Ingest Job ID:" << targetJob.id
    //              << "| Module:" << targetJob.moduleName
    //              << "| Status:" << jobStatus;
        
    //     QVERIFY2(jobStatus == "OK", QString("Ingest Job for Module '%1' has Status '%2'").arg(module).arg(jobStatus).toStdString().c_str());

    //     if (module == "DetectMIMEType")
    //     {
    //         qDebug() << "DetectMIMEType Job is Present in Database with Status:" << jobStatus;
    //     }
    //     else if (module == "TxtCounter")
    //     {
    //         QList<IngestResultItemData> rootItems = tst_CaseManagerInstance->getIngestResultItemsForJob(targetJob.id);
            
    //         QVERIFY2(rootItems.size() > 0, QString("No Result Items for Module: %1").arg(module).toStdString().c_str());
            
    //         for (int rootIdx = 0; rootIdx < rootItems.size(); ++rootIdx) 
    //         {
    //             const IngestResultItemData& rootItem = rootItems[rootIdx];
    //             QString resultType;
    //             switch (rootItem.itemType)
    //             {
    //                 case IngestResultItemData::ItemType::Folder: 
    //                     resultType = "Folder"; 
    //                     break;
    //                 case IngestResultItemData::ItemType::File: 
    //                     resultType = "File"; 
    //                     break;
    //                 case IngestResultItemData::ItemType::Artifact: 
    //                     resultType = "Artifact"; 
    //                     break;
    //                 case IngestResultItemData::ItemType::Text: 
    //                     resultType = "Text"; 
    //                     break;
    //                 case IngestResultItemData::ItemType::Report: 
    //                     resultType = "Report"; 
    //                     break;
    //                 default: 
    //                     resultType = "Unknown"; 
    //                     break;
    //             }
    //             qDebug() << "[Root" << (rootIdx + 1) << "/" << rootItems.size() << "]"
    //                      << "Name:" << rootItem.itemName
    //                      << "| Type:" << resultType
    //                      << "| ID:" << rootItem.id;
                
    //             QList<IngestResultItemData> childItems = tst_CaseManagerInstance->getIngestResultChildren(rootItem.id);
                
    //             QVERIFY2(childItems.size() > 0, QString("Root Folder '%1' has no Child Items.").arg(rootItem.itemName).toStdString().c_str());
                
    //             qDebug() << "Child Item(s):" << childItems.size();
                
    //             for (int childIdx = 0; childIdx < childItems.size(); ++childIdx)
    //             {
    //                 const IngestResultItemData& childItem = childItems[childIdx];
    //                 QString childType;
    //                 switch (childItem.itemType)
    //                 {
    //                     case IngestResultItemData::ItemType::Folder: 
    //                         childType = "Folder"; 
    //                         break;
    //                     case IngestResultItemData::ItemType::File: 
    //                         childType = "File"; 
    //                         break;
    //                     case IngestResultItemData::ItemType::Artifact: 
    //                         childType = "Artifact"; 
    //                         break;
    //                     case IngestResultItemData::ItemType::Text: 
    //                         childType = "Text"; 
    //                         break;
    //                     case IngestResultItemData::ItemType::Report: 
    //                         childType = "Report"; 
    //                         break;
    //                     default: 
    //                         childType = "Unknown"; 
    //                         break;
    //                 }
    //                 qDebug() << "[" << (childIdx + 1) << "/" << childItems.size() << "]"
    //                          << "Name:" << childItem.itemName
    //                          << "| Type:" << childType
    //                          << "| FileId:" << childItem.fileId;
    //             }
    //         }
    //     }
        
    //     jobsValidated++;
    // }
    
    // QVERIFY2(jobsValidated == INGEST_MODULES.size(), QString("Not All Ingest Jobs Validated. %1/%2").arg(jobsValidated).arg(INGEST_MODULES.size()).toStdString().c_str());
}

void TEST_IngestDisk::TEST_ConcurrentTxtCounterJobs()
{

    tst_DataSource.name = "Ingest Disk - 0";
    tst_DataSource.diskSize = 0;
    tst_DataSource.sectorSize = 512;
    tst_DataSource.hash = "";
    tst_DataSource.hashType = "SHA256";
    tst_DataSource.format = "RAW";
    tst_DataSource.filePath = TEST_DISK_PATH;
    tst_DataSource.timeZone = "UTC";
    tst_DataSource.notes = "Test Disk for Concurrent Ingestion";
    tst_DataSource.indexed = 0;

    QVERIFY2(tst_CaseManagerInstance->saveDataSource(tst_DataSource), "Failed to Save Data Source");
    
    QList<DataSourceData> DataSourceList = tst_CaseManagerInstance->getDataSources();
    for (const DataSourceData& ds : DataSourceList) 
    {
        if (ds.filePath == TEST_DISK_PATH && ds.name == tst_DataSource.name) 
        {
            tst_DataSource = ds;
            break;
        }
    }
    
    QVERIFY2(tst_DataSource.id > 0, "Data Source not Assigned an ID in Database");
    QVERIFY2(tst_JobManagerInstance != nullptr, "JobManager not Initialized");
    
    qDebug() << "Concurrent TxtCounter Benchmark for:" << tst_DataSource.name;
    qDebug() << "Disk Path:" << tst_DataSource.filePath;


    QSignalSpy ingestFinishedSpy(tst_JobManagerInstance, SIGNAL(jobFinished(QString, Job::JobType, QString, QString, qint64)));
    QSignalSpy jobFailedSpy(tst_JobManagerInstance, SIGNAL(jobFailed(QString, QString, Job::JobType, QString, QString, qint64)));

    tst_JobManagerInstance->CreateIngestJob(tst_DataSource, tst_CaseInfo.casePath, "TxtCounter");
    qDebug() << "Started Ingest Job 1 for Module: TxtCounter";
    
    tst_JobManagerInstance->CreateIngestJob(tst_DataSource, tst_CaseInfo.casePath, "TxtCounter");
    qDebug() << "Started Ingest Job 2 for Module: TxtCounter";
    
    int expectedJobCount = 2;
    
    while (ingestFinishedSpy.count() < expectedJobCount && jobFailedSpy.count() == 0) 
    {
        ingestFinishedSpy.wait(60000);
    }
    
    QVERIFY2(jobFailedSpy.count() == 0, QString("TxtCounter Concurrent Jobs Failed: %1").arg(jobFailedSpy.count()).toStdString().c_str());
    QVERIFY2(ingestFinishedSpy.count() >= expectedJobCount, QString("TxtCounter Concurrent Jobs Timeout. Expected: %1, Got: %2").arg(expectedJobCount).arg(ingestFinishedSpy.count()).toStdString().c_str());

    
    qDebug() << "Concurrent TxtCounter Jobs Completed Successfully";
}




QTEST_MAIN(TEST_IngestDisk)
#include "tst_ingestDisk.moc"