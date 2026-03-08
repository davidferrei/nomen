#ifndef FSINDEXREPOSITORY_H
#define FSINDEXREPOSITORY_H

#include <QString>
#include <QVector>
#include <QPair>
#include <QByteArray>
#include <QDir>
#include <cstring>

extern "C" {
#include <liblmdb/lmdb.h>
}

#include "data/index/nodeMetaData.h"


class FSIndexRepository
{
public:
    explicit FSIndexRepository(const QString &lmdbPath);
    ~FSIndexRepository();

    bool open();
    void close();

    bool getNodeByPath(const QString &path, uint64_t &outNodeId);
    bool getNode(uint64_t nodeId, NodeMetadata &out);
    QVector<uint64_t> getChildren(uint64_t parentId);
    bool getPathForNode(uint64_t nodeId, QString &outPath);

private:
    QString path;
    MDB_env *env;
};

#endif // FSINDEXREPOSITORY_H
