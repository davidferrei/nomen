

#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDateTime>

#include "core/caseManager/casemanager.h"
#include "core/jobManager/jobManager.h"
#include "core/index/repo/IndexDBManager.h"
#include "core/index/engine/lmdbIndexEngine.h"
#include "data/case/caseInformationData.h"

class TEST_IndexDisk : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    // Tests
    void TEST_CreateAndOpenCase();
    void TEST_CreateIndexJob();
    void TEST_ValidateIndexDB();
    void TEST_ConcurrentIndexJobs();

private:
    QTemporaryDir tst_CaseDir;
    CaseManager* tst_CaseManagerInstance = nullptr;
    JobManager* tst_JobManagerInstance = nullptr;
    CaseInfo tst_CaseInfo;
    DataSourceData tst_DataSource;
    
    const QString TEST_DISK_PATH = "/mnt/disk01/DiskImage/kali.raw";

    QStringList TEST_NODES = 
    {
        "2:0:/etc/passwd",
        "2:0:/etc/group", 
        "2:0:/etc/hostname",
        "2:0:/root",
        "2:0:/home"
    };

    void validateDataSourceIndexed()
    {
        QList<DataSourceData> dataSourceList = tst_CaseManagerInstance->getDataSources();
        bool wasFound = false;
        for (const DataSourceData& ds : dataSourceList) 
        {
            if (ds.filePath == tst_DataSource.filePath && ds.indexed) 
            {
                wasFound = true;
                break;
            }
        }
        QVERIFY2(wasFound, "DataSource not Found or Not Marked as Indexed");
    }
};

//###############################################################################################

void TEST_IndexDisk::initTestCase()
{
    setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
    setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
    setenv("FIREBIRD_LOCK", "/tmp/firebird_lock", 1);
    
    qDebug() << "Test Suite: TEST_IndexDisk Started:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << "Test Case Path:" << tst_CaseDir.path();
    qDebug() << "Test Disk:" << TEST_DISK_PATH;

    tst_CaseManagerInstance = new CaseManager();
    tst_JobManagerInstance = new JobManager();
    
    QVERIFY(tst_CaseDir.isValid());
    QVERIFY2(QFile::exists(TEST_DISK_PATH), "Test Disk Not Found");
}

void TEST_IndexDisk::cleanupTestCase()
{
    qDebug() << "Test Suite: TEST_IndexDisk Finished:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    
    if (tst_CaseManagerInstance != nullptr) 
    {
        delete tst_CaseManagerInstance;
    }

    if (tst_JobManagerInstance != nullptr) 
    {
        delete tst_JobManagerInstance;
    }
}

void TEST_IndexDisk::init()
{
    qDebug() << "Test Started:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void TEST_IndexDisk::cleanup()
{
    qDebug() << "Test Finished:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

//###############################################################################################

void TEST_IndexDisk::TEST_CreateAndOpenCase()
{
    tst_CaseInfo.caseId = "TEST_INDEX_CASE_001";
    tst_CaseInfo.caseName = "Index Test Case";
    tst_CaseInfo.createdDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    tst_CaseInfo.updatedDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    tst_CaseInfo.notes = "Test Case for Disk Indexing";
    tst_CaseInfo.caseState = 0;
    tst_CaseInfo.casePath = tst_CaseDir.path() + "/index_test_case";
    
    QVERIFY2(tst_CaseManagerInstance->createCaseInfo(tst_CaseInfo), "Failed to Create Case Info");
       
    QVERIFY2(tst_CaseManagerInstance->openCase(tst_CaseInfo.casePath), "Failed to Open Case");
    
    CaseInfo& openedCase = tst_CaseManagerInstance->getCurrentCase();
    QCOMPARE(openedCase.caseId, tst_CaseInfo.caseId);
    
    qDebug() << "Case Successfully Created and Opened for Indexing Tests";
}

void TEST_IndexDisk::TEST_CreateIndexJob()
{
    QVERIFY(!tst_CaseInfo.casePath.isEmpty());

    tst_DataSource.name = "Indexing Disk - 0";
    tst_DataSource.diskSize = 0;
    tst_DataSource.sectorSize = 512;
    tst_DataSource.hash = "";
    tst_DataSource.hashType = "SHA256";
    tst_DataSource.format = "RAW";
    tst_DataSource.filePath = TEST_DISK_PATH;
    tst_DataSource.timeZone = "UTC";
    tst_DataSource.notes = "Test Disk for Indexing";
    tst_DataSource.indexed = 0;

    QVERIFY2(tst_CaseManagerInstance->saveDataSource(tst_DataSource), "Failed to Save Data Source");
    
    QList<DataSourceData> DataSourceList = tst_CaseManagerInstance->getDataSources();
    for (const DataSourceData& ds : DataSourceList) 
    {
        if (ds.filePath == TEST_DISK_PATH) 
        {
            tst_DataSource = ds;
            break;
        }
    }
    
    QVERIFY2(tst_DataSource.id > 0, "Data Source not Assigned an ID in Database");
    QVERIFY2(tst_JobManagerInstance != nullptr, "JobManager not Initialized");
    
    qDebug() << "Index Job Created for:" << tst_DataSource.name;
    qDebug() << "Disk Path:" << tst_DataSource.filePath;
    qDebug() << "Index Location:" << tst_CaseInfo.casePath + "/indexes/disk-" + QString::number(tst_DataSource.id);

    QBENCHMARK 
    {
        tst_JobManagerInstance->CreateIndexJob(tst_DataSource, tst_CaseInfo.casePath);
        
        QSignalSpy indexFinishedSpy(tst_JobManagerInstance, SIGNAL(jobFinished(QString, Job::JobType, QString, QString, qint64)));
        QSignalSpy jobFailedSpy(tst_JobManagerInstance, SIGNAL(jobFailed(QString, QString, Job::JobType, QString, QString, qint64)));
        bool indexCompleted = indexFinishedSpy.wait(600000);
        
        QVERIFY2(jobFailedSpy.count() == 0, QString("Index Job Failed: %1").arg(jobFailedSpy.count()).toStdString().c_str());
        QVERIFY2(indexCompleted, "Index Job Timeout");
    }
    
    qDebug() << "Index Completed Successfully";
    
    validateDataSourceIndexed();
}

void TEST_IndexDisk::TEST_ValidateIndexDB()
{
    QVERIFY(!tst_CaseInfo.casePath.isEmpty());
    
    QList<DataSourceData> dataSourceList = tst_CaseManagerInstance->getDataSources();
    QVERIFY2(dataSourceList.size() > 0, "No Data Sources Found");
    
    DataSourceData targetDS;
    for (const DataSourceData& ds : dataSourceList) 
    {
        if (ds.filePath == TEST_DISK_PATH) 
        {
            targetDS = ds;
            break;
        }
    }
    
    QVERIFY2(targetDS.id > 0, "Test Data Source not Found");

    QString indexPath = tst_CaseInfo.casePath + "/indexes/disk-" + QString::number(targetDS.id);
    QVERIFY2(QDir(indexPath).exists(), QString("Index directory Not Found: %1").arg(indexPath).toStdString().c_str());
    
    qDebug() << "Data Source Indexed:" << targetDS.name;
    qDebug() << "Index Path:" << indexPath;
       
    QVERIFY2(QFile::exists(indexPath + "/data.mdb"), QString("LMDB Data File Not Found: %1").arg(indexPath + "/data.mdb").toStdString().c_str());
    QVERIFY2(QFile::exists(indexPath + "/lock.mdb"), QString("LMDB Lock File Not Found: %1").arg(indexPath + "/lock.mdb").toStdString().c_str());
    
    
    IndexDBManager indexDBManagerInstance(indexPath);
    QVERIFY2(indexDBManagerInstance.open(), "Failed to Open LMDB database");
    
    qDebug() << "Successfully Opened LMDB Database";
    
    // lmdbIndexEngine lmdbDebug;
    // if (lmdbDebug.initialize(indexPath)) 
    // {
    //     lmdbDebug.debug_dumpLmdbFirst20(indexPath);
    //     lmdbDebug.debug_dumpPathsRevFirst20(indexPath);
    //     lmdbDebug.debug_dumpSpecialNodes(indexPath);
    //     lmdbDebug.close();
    // }
     
    int nodesFound = 0;
    for (const QString& node : TEST_NODES) 
    {
        auto [isSuccess, nodeId] = indexDBManagerInstance.getNodeByPath(node);
        
        if (!isSuccess) 
        {
            qWarning() << "Not Found:" << node;
            continue;
        }

        nodesFound++;
        auto [isMetadataSuccess, metadata] = indexDBManagerInstance.getNode(nodeId);
        
        if (!isMetadataSuccess) 
        {
            qWarning() << "Found Node but Failed to get Metadata for:" << node;
            continue;
        }

        QString nodeType = metadata.is_dir ? "Directory" : "File";
        QString nodeStatus;

        switch (metadata.status) 
        {
            case 0: 
                nodeStatus = "Allocated"; 
                break;
            case 1: 
                nodeStatus = "Unallocated"; 
                break;
            case 2: 
                nodeStatus = "Deleted"; 
                break;
            case 3: 
                nodeStatus = "Orphan"; 
                break;
            case 4: 
                nodeStatus = "Raw"; 
                break;
            default: 
                nodeStatus = "Unknown"; 
                break;
        }
        
        qDebug() << "Found:" << node
                 << "| Type:" << nodeType
                 << "| Status:" << nodeStatus
                 << "| Size:" << metadata.size
                 << "| Inode:" << metadata.inode;

    }
    
    QVERIFY2(nodesFound >= TEST_NODES.size(), "Not All Known Nodes Found in the Index - Indexing May Have Failed");
    
    qDebug() << "Found" << nodesFound << "/" << TEST_NODES.size() <<"Known Nodes in the Index.";
}

void TEST_IndexDisk::TEST_ConcurrentIndexJobs()
{
        
    QList<DataSourceData> DataSourceList;
    
    for (int i = 0; i < 4; ++i) 
    {
        DataSourceData ds;
        ds.name = QString("Concurrent Disk %1").arg(i + 1);
        ds.diskSize = 0;
        ds.sectorSize = 512;
        ds.hash = "";
        ds.hashType = "SHA256";
        ds.format = "RAW";
        ds.filePath = TEST_DISK_PATH;
        ds.timeZone = "UTC";
        ds.notes = QString("Concurrent Indexing Test Disk %1").arg(i + 1);
        ds.indexed = 0;
        
        QVERIFY2(tst_CaseManagerInstance->saveDataSource(ds), QString("Failed to Save Data Source %1").arg(i + 1).toStdString().c_str());
        
        QList<DataSourceData> savedDataSources = tst_CaseManagerInstance->getDataSources();
        for (const DataSourceData& savedDs : savedDataSources) 
        {
            if (savedDs.name == ds.name) 
            {
                ds = savedDs;
                break;
            }
        }
        
        QVERIFY2(ds.id > 0, QString("Data Source %1 not Assigned ID").arg(i + 1).toStdString().c_str());
        
        DataSourceList.append(ds);
        
        qDebug() << "Created Data Source" << (i + 1) << "with ID:" << ds.id;
    }
    
    for (int i = 0; i < DataSourceList.size(); ++i) 
    {
        tst_JobManagerInstance->CreateIndexJob(DataSourceList[i], tst_CaseInfo.casePath);

        qDebug() << "Started Index Job" << (i + 1) << "for DataSource ID:" << DataSourceList[i].id;
    }
    
    QSignalSpy indexFinishedSpy(tst_JobManagerInstance, SIGNAL(jobFinished(QString, Job::JobType, QString, QString, qint64)));
    QSignalSpy jobFailedSpy(tst_JobManagerInstance, SIGNAL(jobFailed(QString, QString, Job::JobType, QString, QString, qint64)));
    
    int expectedJobCount = 4;
    bool allJobsCompleted = false;
    
    while (indexFinishedSpy.count() < expectedJobCount && jobFailedSpy.count() == 0) 
    {
        allJobsCompleted = indexFinishedSpy.wait(600000);
        if (!allJobsCompleted && jobFailedSpy.count() > 0) break;
    }
    
    QVERIFY2(jobFailedSpy.count() == 0, QString("Concurrent Jobs Failed: %1").arg(jobFailedSpy.count()).toStdString().c_str());
    QVERIFY2(indexFinishedSpy.count() >= expectedJobCount, QString("Concurrent Jobs Timeout. Expected: %1, Got: %2").arg(expectedJobCount).arg(indexFinishedSpy.count()).toStdString().c_str());
    
    qDebug() << "All 4 Concurrent Index Jobs Completed Successfully!";
}

QTEST_MAIN(TEST_IndexDisk)
#include "tst_indexDisk.moc"
