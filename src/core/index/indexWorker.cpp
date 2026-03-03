#include "indexWorker.h"

IndexWorker::IndexWorker(const DataSourceData& ds, const QString& casePath, QObject* parent) : QObject(parent), dataSource(ds), caseFolderPath(casePath) {}

void IndexWorker::run()
{
    qDebug() << "[IndexWorker] Indexing started for:" << dataSource.filePath;

    try
    {
        QString lmdbPath = caseFolderPath + "/indexes/" + dataSource.name;

        LMDBManager lmdb;
        if (!lmdb.initialize(lmdbPath)) {
            emit failed("Failed to initialize LMDB at " + lmdbPath);
            return;
        }

        lmdb.writeTestData();
        lmdb.readTestData();

        // Simulate indexing
        // QThread::sleep(10);

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
