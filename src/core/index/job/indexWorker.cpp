#include "indexWorker.h"

IndexWorker::IndexWorker(const DataSourceData& ds, const QString& casePath, QObject* parent) : QObject(parent), dataSource(ds), caseFolderPath(casePath) {}

void IndexWorker::run()
{
    qDebug() << "[IndexWorker] Indexing started for:" << dataSource.filePath;

    try
    {
        CaseDBThreadInstance dbThread(caseFolderPath);
        if (!dbThread.openDatabase()) 
        {
            qWarning() << "IndexWorker: Failed to Open Case DB for" << dataSource.filePath;
            emit failed("Failed to Open Case DB");
            return;
        }

        QString lmdbPath = caseFolderPath + "/indexes/" + "disk-" % QString::number(dataSource.id);

        lmdbIndexEngine lmdb;

        if (!lmdb.initialize(lmdbPath)) 
        {
            emit failed("Failed to Initialize LMDB at " + lmdbPath);
            return;
        }

        if (!TskWalker::walkImage(dataSource.filePath, lmdb)) 
        {
            emit failed("TSK Disk Walk Failed for " + dataSource.filePath);
            return;
        }

        #ifdef QT_DEBUG
            lmdb.debug_dumpAllLmdbTest(lmdbPath);
            lmdb.debug_dumpLmdbFirst20(lmdbPath);
            lmdb.debug_dumpSpecialNodes(lmdbPath);
        #endif
        
        DataSourceRepository DSRepo = dbThread.dataSourceRepo();
        DataSourceData updatedDS = dataSource;
        updatedDS.indexed = true;
        if (updatedDS.id > 0) 
        {
            if (!DSRepo.update(updatedDS.id, updatedDS)) 
            {
                qWarning() << "IndexWorker: Failed to Update Datasource for:" << updatedDS.id;
                emit failed("Failed to Update Datasource in DB");
                return;
            }
        } 
        else 
        {
            if (!DSRepo.insert(updatedDS)) 
            {
                qWarning() << "IndexWorker: Failed to Insert Datasource for:" << updatedDS.id;
                emit failed("Failed to Insert Datasource in DB");
                return;
            }
        }

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
