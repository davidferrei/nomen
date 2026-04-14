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

//TODO: Make this a QObject 

class IndexDBManager
{
public:
    explicit IndexDBManager(const QString &lmdbPath);
    ~IndexDBManager();

    bool open();
    void close();

    //Getters
    QPair<bool, quint64> getNodeByPath(const QString &path);
    QPair<bool, NodeMetadata> getNode(quint64 nodeId);
    QPair<bool, QVector<quint64>> getChildren(quint64 parentId);
    QPair<bool, QString> getPathForNode(quint64 nodeId);
    QPair<bool, QVector<quint64>> getChildDirectories(quint64 parentId);
    QPair<bool, quint64> getNodeByInode(quint64 part, quint64 fsIdx, quint64 inode);


    //Debug
    void debugListChildrenInDb(quint64 parentId);


private:
    QString path;
    MDB_env *env;

    MDB_dbi paths_dbi = 0;
    MDB_dbi paths_rev_dbi = 0;
    MDB_dbi nodes_dbi = 0;
    MDB_dbi children_dbi = 0;
    MDB_dbi inode_map_dbi = 0;
};

#endif // INDEXDBMANAGER_H
