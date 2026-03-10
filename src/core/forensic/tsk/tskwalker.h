#ifndef TSKWALKER_H
#define TSKWALKER_H

#include <tsk/libtsk.h>

#include "../../lmdb/lmdbManager.h"

#include "data/index/nodeMetaData.h"


#include <QString>
#include <QDebug>
#include <QList>

#include <cstring>
#include <vector>
#include <unordered_map>


class LMDBManager;

struct WalkContext 
{
    MDB_txn* lmdb_txn;
    MDB_dbi lmdb_nodes_dbi;
    MDB_dbi lmdb_children_dbi;
    MDB_dbi lmdb_paths_dbi;
    MDB_dbi lmdb_inode_map_dbi;
    uint64_t next_node_id;
    uint64_t file_count;
    uint64_t dir_count;
    int partitionId;
    int fsIndex;
    MDB_env* lmdb_env;
    uint64_t lmdb_commit_batch;
};

class TskWalker
{
public:
    static bool walkImage(const QString &imagePath, LMDBManager &lmdb);
};

#endif // TSKWALKER_H
