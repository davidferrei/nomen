#include "ingestWorker.h"


IngestWorker::IngestWorker(const DataSourceData& ds, const QString& casePath, QObject* parent) : QObject(parent), dataSource(ds), caseFolderPath(casePath) {}

void IngestWorker::run()
{
    qDebug() << "[IngestWorker] Starting Ingest for:" << dataSource.filePath << " with Module:" << moduleName;

    PerformanceTimer perfTimer(QString("Ingest Time - (%1)").arg(moduleName));

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

        module->init();

        if (auto *tskModule = dynamic_cast<TSKIngestModule*>(module)) 
        {
            if (!tskModule->runWalk(dataSource.filePath)) 
            {
                qWarning() << "[IngestWorker] TSK Walk Failed for Module" << moduleName;
                emit failed(QString("TSK Walk failed for %1").arg(moduleName));
                return;
            }
        } 
        else 
        {
            module->process();
        }

        qDebug() << "[IngestWorker] Ingest Module Finished For:" << dataSource.filePath;
        emit finished();
    }
    catch (const std::exception &e) 
    {
        qWarning() << "[IngestWorker] Exception:" << e.what();
        emit failed(QString::fromUtf8(e.what()));
    }
    catch (...) 
    {
        qWarning() << "[IngestWorker] Unknown Error";
        emit failed("Unknown Error");
    }
}
