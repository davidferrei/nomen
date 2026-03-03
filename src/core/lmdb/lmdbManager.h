#ifndef LMDBMANAGER_H
#define LMDBMANAGER_H

#include <QDebug>
#include <QString>
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
    bool writeTestData();
    bool readTestData();
    void close();

private:
    MDB_env* env;
    MDB_dbi dbi;
    bool initialized;
};

#endif // LMDBMANAGER_H
