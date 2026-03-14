#include "indexWorker.h"
#include "core/forensic/tsk/tskwalker.h"

IndexWorker::IndexWorker(const DataSourceData& ds, const QString& casePath, QObject* parent) : QObject(parent), dataSource(ds), caseFolderPath(casePath) {}

void IndexWorker::run()
{
    qDebug() << "[IndexWorker] Indexing started for:" << dataSource.filePath;

    try
    {
        QString lmdbPath = caseFolderPath + "/indexes/" + "disk-" % QString::number(dataSource.id);

        lmdbIndexEngine lmdb;
        
        if (!lmdb.initialize(lmdbPath)) {
            emit failed("Failed to Initialize LMDB at " + lmdbPath);
            return;
        }

        if (!TskWalker::walkImage(dataSource.filePath, lmdb)) {
            emit failed("TSK Disk Walk Failed for " + dataSource.filePath);
            return;
        }

        #ifdef QT_DEBUG
            lmdb.debug_dumpAllLmdbTest(lmdbPath);
            lmdb.debug_dumpLmdbFirst20(lmdbPath);
            lmdb.debug_dumpSpecialNodes(lmdbPath);
        #endif
        
        //FIXME: Save Ingested = True to DS in DB

        qDebug() << "[IndexWorker] Indexing Finished for:" << dataSource.filePath;
        emit finished();
    }
    catch (const std::exception& e)
    {
        qWarning() << "[IndexWorker] Caught Exception for:" << dataSource.filePath<< "Error:" << e.what();
        emit failed(QString::fromUtf8(e.what()));
    }
    catch (...)
    {
        qWarning() << "[IndexWorker] Unknown Error for:" << dataSource.filePath;
        emit failed("Unknown Critical Error");
    }

}
