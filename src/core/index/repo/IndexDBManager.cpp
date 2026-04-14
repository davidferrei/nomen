#include "IndexDBManager.h"

IndexDBManager::IndexDBManager(const QString &lmdbPath) : path(lmdbPath), env(nullptr) {}

IndexDBManager::~IndexDBManager()
{
    close();
}

//############################################################################################
// Functions

static inline uint64_t read_u64_from_val(const MDB_val &v)
{
    if (v.mv_size < (int)sizeof(uint64_t)) return 0;
    uint64_t x = 0;
    memcpy(&x, v.mv_data, sizeof(uint64_t));
    return x;
}


bool IndexDBManager::open()
{
    if (env) return true;

    if (!QDir().exists(path)) 
    {
        qWarning() << "[IndexDBManager]: LMDB Path Does not Exist:" << path;
        return false;
    }

    if (mdb_env_create(&env) != 0) 
    {
        qWarning() << "[IndexDBManager]: LMDB: env_create Failed";
        env = nullptr;
        return false;
    }
    mdb_env_set_maxdbs(env, 16);

    int rc = mdb_env_open(env, path.toUtf8().constData(), MDB_RDONLY, 0664);
    if (rc != 0) 
    {
        qWarning() << "[IndexDBManager]: LMDB: env_open Failed:" << mdb_strerror(rc);
        mdb_env_close(env);
        env = nullptr;
        return false;
    }

    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) == 0)
    {
        if (mdb_dbi_open(txn, "paths", 0, &paths_dbi) != 0) paths_dbi = 0;
        if (mdb_dbi_open(txn, "paths_rev", 0, &paths_rev_dbi) != 0) paths_rev_dbi = 0;
        if (mdb_dbi_open(txn, "nodes", 0, &nodes_dbi) != 0) nodes_dbi = 0;
        if (mdb_dbi_open(txn, "children", 0, &children_dbi) != 0) children_dbi = 0;
        if (mdb_dbi_open(txn, "inode_map", 0, &inode_map_dbi) != 0) inode_map_dbi = 0;

        
        mdb_txn_commit(txn);
    }

    qInfo() << "[IndexDBManager]: LMDB FS Index Repository Opened:" << path;
    return true;
}

void IndexDBManager::close()
{
    if (!env) return;

    if (paths_dbi) mdb_dbi_close(env, paths_dbi);
    if (paths_rev_dbi) mdb_dbi_close(env, paths_rev_dbi);
    if (nodes_dbi) mdb_dbi_close(env, nodes_dbi);
    if (children_dbi) mdb_dbi_close(env, children_dbi);
    if (inode_map_dbi) mdb_dbi_close(env, inode_map_dbi);

    paths_dbi = paths_rev_dbi = nodes_dbi = children_dbi = inode_map_dbi = 0;

    mdb_env_close(env);
    env = nullptr;
}

//############################################################################################
// Getters 

QPair<bool, quint64> IndexDBManager::getNodeByPath(const QString &p)
{
    if (!env || !paths_dbi) return QPair<bool, quint64>(false, 0);

    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) return QPair<bool, quint64>(false, 0);

    QByteArray key = p.toUtf8();
    MDB_val k{ static_cast<size_t>(key.size()), const_cast<char*>(key.data()) };
    MDB_val v{};
    
    int rc = mdb_get(txn, paths_dbi, &k, &v);
    if (rc != 0) 
    {
        mdb_txn_abort(txn);
        return QPair<bool, quint64>(false, 0);
    }

    quint64 nid = (quint64)read_u64_from_val(v);
    mdb_txn_abort(txn);

    return QPair<bool, quint64>(true, nid);
}

QPair<bool, NodeMetadata> IndexDBManager::getNode(quint64 nodeId)
{
    if (!env || !nodes_dbi) return QPair<bool, NodeMetadata>(false, NodeMetadata());

    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) return QPair<bool, NodeMetadata>(false, NodeMetadata());

    MDB_val k{ sizeof(nodeId), &nodeId };
    MDB_val v{};
    if (mdb_get(txn, nodes_dbi, &k, &v) != 0) 
    { 
        mdb_txn_abort(txn); 
        return QPair<bool, NodeMetadata>(false, NodeMetadata());
    }

    NodeMetadata out{};
    if (v.mv_size >= (int)sizeof(out)) 
    {
        memcpy(&out, v.mv_data, sizeof(out));
        mdb_txn_abort(txn);
        return QPair<bool, NodeMetadata>(true, out);
    }

    mdb_txn_abort(txn);
    return QPair<bool, NodeMetadata>(false, NodeMetadata());
}

QPair<bool, QVector<quint64>> IndexDBManager::getChildren(quint64 parentId)
{
    QVector<quint64> out;
    if (!env || !children_dbi) return QPair<bool, QVector<quint64>>(false, out);

    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) return QPair<bool, QVector<quint64>>(false, out);

    MDB_cursor *cursor = nullptr;
    if (mdb_cursor_open(txn, children_dbi, &cursor) != 0) 
    { 
        mdb_txn_abort(txn); 
        return QPair<bool, QVector<quint64>>(false, out); 
    }

    MDB_val k{ sizeof(parentId), &parentId };
    MDB_val v{};

    int rc = mdb_cursor_get(cursor, &k, &v, MDB_SET);
    if (rc == 0) 
    {
        do {
            uint64_t child = read_u64_from_val(v);
            out.append((quint64)child);
            rc = mdb_cursor_get(cursor, &k, &v, MDB_NEXT_DUP);

        } while (rc == 0);
    }

    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);

    return QPair<bool, QVector<quint64>>(true, out);
}



QPair<bool, QString> IndexDBManager::getPathForNode(quint64 nodeId)
{
    if (!env || !paths_rev_dbi) return QPair<bool, QString>(false, QString());

    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0)
        return QPair<bool, QString>(false, QString());

    MDB_val k{ sizeof(nodeId), &nodeId };
    MDB_val v{};
    int rc = mdb_get(txn, paths_rev_dbi, &k, &v);
    if (rc != 0)
    {
        mdb_txn_abort(txn);
        return QPair<bool, QString>(false, QString());
    }

    QString outPath = QString::fromUtf8(reinterpret_cast<const char*>(v.mv_data), (int)v.mv_size);
    mdb_txn_abort(txn);
    return QPair<bool, QString>(true, outPath);
}

QPair<bool, QVector<quint64>> IndexDBManager::getChildDirectories(quint64 parentId)
{
    QVector<quint64> out;
    if (!env) return QPair<bool, QVector<quint64>>(false, out);

    auto childrenResult = getChildren(parentId);
    if (!childrenResult.first)
        return QPair<bool, QVector<quint64>>(false, out);

    for (quint64 cid : childrenResult.second)
    {
        auto metaResult = getNode(cid);
        if (metaResult.first && metaResult.second.is_dir)
            out.append(cid);
    }
    return QPair<bool, QVector<quint64>>(true, out);
}

QPair<bool, quint64> IndexDBManager::getNodeByInode(quint64 part, quint64 fsIdx, quint64 inode)
{
    if (!env || !inode_map_dbi) return QPair<bool, quint64>(false, 0);

    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) return QPair<bool, quint64>(false, 0);

    unsigned char ikbuf[24];
    memcpy(ikbuf, &part, sizeof(part));
    memcpy(ikbuf + sizeof(part), &fsIdx, sizeof(fsIdx));
    memcpy(ikbuf + sizeof(part) + sizeof(fsIdx), &inode, sizeof(inode));

    MDB_val k{ sizeof(ikbuf), ikbuf };
    MDB_val v{};

    int rc = mdb_get(txn, inode_map_dbi, &k, &v);
    if (rc != 0) 
    { 
        mdb_txn_abort(txn); 
        return QPair<bool, quint64>(false, 0);
    }

    quint64 nid = (quint64)read_u64_from_val(v);
    mdb_txn_abort(txn);
    
    return QPair<bool, quint64>(true, nid);
}


//############################################################################################
// Debug 

void IndexDBManager::debugListChildrenInDb(quint64 parentId)
{
    if (!env) 
    {
        qWarning() << "[IndexDBManager] debugListChildrenInDb: env Not Open";
        return;
    }

    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) 
    {
        qWarning() << "[IndexDBManager] debugListChildrenInDb: txn Begin Failed";
        return;
    }

    MDB_dbi children_dbi = 0;
    if (mdb_dbi_open(txn, "children", 0, &children_dbi) != 0) children_dbi = 0;
    if (!children_dbi) 
    { 
        mdb_txn_abort(txn); 
        qWarning() << "[IndexDBManager] debugListChildrenInDb: Children DB Missing"; 
        return; 
    }

    MDB_cursor *cursor = nullptr;
    if (mdb_cursor_open(txn, children_dbi, &cursor) != 0) 
    { 
        mdb_txn_abort(txn); 
        qWarning() << "[IndexDBManager] debugListChildrenInDb: cursor Open Failed"; 
        return; 
    }

    MDB_val k{ sizeof(parentId), &parentId };
    MDB_val v{};
    int rc = mdb_cursor_get(cursor, &k, &v, MDB_SET);
    if (rc != 0) 
    {
        qInfo() << "[IndexDBManager] debugListChildrenInDb: no Children for Parent" << parentId;
        mdb_cursor_close(cursor);
        mdb_txn_abort(txn);

        return;
    }

    qInfo() << "[IndexDBManager] debugListChildrenInDb: Children in DB for Parent" << parentId << ":";

    do {
        uint64_t child = read_u64_from_val(v);
        qInfo() << "  ->" << child;
        rc = mdb_cursor_get(cursor, &k, &v, MDB_NEXT_DUP);

    } while (rc == 0);

    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);

}


