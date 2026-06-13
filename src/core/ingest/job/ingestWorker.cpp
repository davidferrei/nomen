#include "ingestWorker.h"

#include <QList>
// Debug Helpers
static void debugPrintJob(const IngestJobHistoryData &job)
{
    qDebug() << "[DEBUG] IngestJob ID:" << job.id
             << "Module:" << job.moduleName
             << "Version:" << job.moduleVersion
             << "Start:" << job.startTime.toString(Qt::ISODate)
             << "End:" << job.endTime.toString(Qt::ISODate)
             << "Status:" << job.status
             << "Log length:" << job.log.size();
}

static void debugPrintAllJobs(IngestJobHistoryRepository repo)
{
    QList<IngestJobHistoryData> list = repo.getAll();
    qDebug() << "[DEBUG] IngestJobs count:" << list.size();
    for (const IngestJobHistoryData &j : list) {
        debugPrintJob(j);
    }
}

IngestWorker::IngestWorker(const DataSourceData& ds, const QString& casePath, const QString& moduleNameArg, QObject* parent) : QObject(parent), dataSource(ds), caseFolderPath(casePath), moduleName(moduleNameArg) {}

void IngestWorker::run()
{

    qDebugNL();
    qDebug() << "[IngestWorker] Starting Ingest for:" << dataSource.filePath << " with Module:" << moduleName;

    // qDebug() << "[IngestWorker] Preparing CaseDBThreadInstance and Repository Access";

    PerformanceTimer perfTimer(QString("Ingest Time - (%1)").arg(moduleName));

    CaseDBThreadInstance dbThread(caseFolderPath);
    // qDebug() << "[IngestWorker] Created CaseDBThreadInstance for:" << caseFolderPath;
    IngestJobHistoryData ingestJobData;


    try 
    {
        // qDebug() << "[IngestWorker] Attempting to Open Case DB";
        if (!dbThread.openDatabase()) 
        {
            qWarning() << "[IngestWorker]: Failed to Open Case DB for" << dataSource.filePath;
            emit failed("Failed to Open Case DB");
            return;
        }
        // qDebug() << "[IngestWorker] Case DB opened Successfully";
    }
    catch(const std::exception& e)
    {
        qWarning() << "[IngestWorker] Exception while Opening DB:" << e.what();
        emit failed("Failed to Open Case DB");
        return;
    }
    
    IngestResultDBInstance *ingestResultDB = nullptr;

    debugPrintAllJobs(dbThread.ingestJobsRepo());

    try 
    {

        IngestModuleFactory &moduleFactory = IngestModuleFactory::instance();

        AbstractIngestModule *module = moduleFactory.fetchModule(moduleName);
        if (!module) 
        {
            qWarning() << "[IngestWorker] Module not Found:" << moduleName;
            emit failed(QString("Module Not Found: %1").arg(moduleName));
            return;
        }

        // Register Job
        ingestJobData.moduleName = moduleName;
        ingestJobData.moduleVersion = moduleFactory.getModuleMetadata(moduleName).value("version").toString();
        ingestJobData.startTime = QDateTime::currentDateTime();
        ingestJobData.status = 1;
        ingestJobData.settingsJson = QString("{}");

        // qDebug() << "[IngestWorker] Ingest Job Prepared (module, status):" << ingestJobData.moduleName << ingestJobData.status;
        if (!dbThread.ingestJobsRepo().insert(ingestJobData)) 
        {
            qWarning() << "[IngestWorker]: Failed to Save Job";
            emit failed("Failed to Insert Job into the DB");
            return;
        }

        debugPrintJob(ingestJobData);

        ingestResultDB = new IngestResultDBInstance(dbThread.database(), ingestJobData.id, this);
        module->onJobStart(ingestResultDB);

        // Run Module
        module->init();

        qDebugNL();

        if (auto *tskModule = dynamic_cast<tskIngestModuleInterface*>(module)) 
        {
            if (!tskModule->runWalk(dataSource.filePath)) 
            {
                qWarning() << "[IngestWorker] TSK Walk Failed for Module" << moduleName;
                emit failed(QString("TSK Walk Failed for %1").arg(moduleName));
                return;
            }
        } 
        else 
        {
            module->process();
        }

        qDebugNL();

        // Update Job History
        ingestJobData.status = 2;
        ingestJobData.log.clear();

        if (!dbThread.ingestJobsRepo().setStatus(ingestJobData.id, ingestJobData.status)) 
        {
            qWarning() << "[IngestWorker]: Failed to Set Job Status";
            emit failed("Failed to Update Job History in DB");
            return;
        }

        debugPrintJob(ingestJobData);
        debugPrintAllJobs(dbThread.ingestJobsRepo());


        emit finished();
    }
    catch (const std::exception &e) 
    {
        qWarning() << "[IngestWorker] Exception:" << e.what();
        ingestJobData.status = 3;
        ingestJobData.log = QString::fromUtf8(e.what());

        qWarning() << "[IngestWorker] Calling setStatus for Failed Job ID:" << ingestJobData.id;
        if (!dbThread.ingestJobsRepo().setStatus(ingestJobData.id, ingestJobData.status)) 
        {
            qWarning() << "[IngestWorker]: Failed to Set Job Status";
            emit failed("Failed to Update Job History in DB");
            return;
        }

        qWarning() << "[IngestWorker] Appending error log for Failed Job ID:" << ingestJobData.id << "log:" << ingestJobData.log;
        if (!dbThread.ingestJobsRepo().appendLog(ingestJobData.id, ingestJobData.log)) 
        {
            qWarning() << "[IngestWorker]: Failed to Append Job Log";
            emit failed("Failed to Update Job History in DB");
            return;
        }
        emit failed(QString::fromUtf8(e.what()));
    }
    catch (...) 
    {
        qWarning() << "[IngestWorker] Unknown Error";
        ingestJobData.status = 3;
        ingestJobData.log = QString("Unknown Error");

        qWarning() << "[IngestWorker] Calling setStatus for Unknown Error Job ID:" << ingestJobData.id;
        if (!dbThread.ingestJobsRepo().setStatus(ingestJobData.id, ingestJobData.status)) 
        {
            qWarning() << "[IngestWorker]: Failed to Set Job Status";
            emit failed("Failed to Update Job History in DB");
            return;
        }

        qWarning() << "[IngestWorker] Appending unknown error log for Unknown Error Job ID:" << ingestJobData.id << "log:" << ingestJobData.log;
        if (!dbThread.ingestJobsRepo().appendLog(ingestJobData.id, ingestJobData.log)) 
        {
            qWarning() << "[IngestWorker]: Failed to Append Job Log";
            emit failed("Failed to Update Job History in DB");
            return;
        }

        emit failed("Unknown Error");
    }
}
