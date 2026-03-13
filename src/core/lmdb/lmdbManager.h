#ifndef LMDBMANAGER_H
#define LMDBMANAGER_H

#include "../logging/performancemeasure.h"
#include "data/index/nodeMetaData.h"


#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QDir>
#include <QDir>


extern "C" {
#include <liblmdb/lmdb.h>
}


class LMDBManager
{
public:
    LMDBManager();
    ~LMDBManager();

    bool initialize(const QString& path);
    void close();

    MDB_env* envHandle() const { return env; }
    
    bool debug_dumpAllLmdbTest(const QString& lmdbPath);
    bool debug_dumpLmdbFirst20(const QString& lmdbPath);
    bool debug_dumpSpecialNodes(const QString& lmdbPath);

private:
    MDB_env* env;
    MDB_dbi dbi;
    bool initialized;
};

#endif // LMDBMANAGER_H
