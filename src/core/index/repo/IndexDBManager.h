#ifndef INDEXDBMANAGER_H
#define INDEXDBMANAGER_H

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


class IndexDBManager
{
public:
    explicit IndexDBManager(const QString &lmdbPath);
    ~IndexDBManager();

    bool open();
    void close();


    QPair<bool, quint64> getNodeByPath(const QString &path);
    QPair<bool, NodeMetadata> getNode(quint64 nodeId);
    QPair<bool, QVector<quint64>> getChildren(quint64 parentId);
    QPair<bool, QString> getPathForNode(quint64 nodeId);
    QPair<bool, quint64> getNodeByInode(quint64 part, quint64 fsIdx, quint64 inode);

    //Getters
    

    //Debug
    void debugListChildrenInDb(quint64 parentId);


private:
    QString path;
    MDB_env *env;
};

#endif // INDEXDBMANAGER_H
