#include "FSIndexRepository.h"

FSIndexRepository::FSIndexRepository(const QString &lmdbPath) : path(lmdbPath), env(nullptr) {}

FSIndexRepository::~FSIndexRepository()
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


bool FSIndexRepository::open()
{
    if (env) return true;

    if (!QDir().exists(path)) {
        qWarning() << "[FSIndexRepository]: LMDB Path Does not Exist:" << path;
        return false;
    }

    if (mdb_env_create(&env) != 0) {
        qWarning() << "[FSIndexRepository]: LMDB: env_create Failed";
        env = nullptr;
        return false;
    }
    mdb_env_set_maxdbs(env, 16);

    int rc = mdb_env_open(env, path.toUtf8().constData(), MDB_RDONLY, 0664);
    if (rc != 0) {
        qWarning() << "[FSIndexRepository]: LMDB: env_open Failed:" << mdb_strerror(rc);
        mdb_env_close(env);
        env = nullptr;
        return false;
    }

    qInfo() << "[FSIndexRepository]: LMDB FS Index Repository Opened:" << path;
    return true;
}

void FSIndexRepository::close()
{
    if (!env) return;
    mdb_env_close(env);
    env = nullptr;
}

//############################################################################################
// Getters 

bool FSIndexRepository::getNodeByPath(const QString &p, uint64_t &outNodeId)
{
    if (!env) return false;
    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) return false;

    MDB_dbi paths_dbi = 0;
    if (mdb_dbi_open(txn, "paths", 0, &paths_dbi) != 0) paths_dbi = 0;
    if (!paths_dbi) {
        mdb_txn_abort(txn);
        return false;
    }

    QByteArray key = p.toUtf8();
    MDB_val k{ static_cast<size_t>(key.size()), const_cast<char*>(key.data()) };
    MDB_val v{};
    int rc = mdb_get(txn, paths_dbi, &k, &v);
    if (rc != 0) {
        mdb_txn_abort(txn);
        return false;
    }

    outNodeId = read_u64_from_val(v);
    mdb_txn_abort(txn);
    return true;
}

bool FSIndexRepository::getNode(uint64_t nodeId, NodeMetadata &out)
{
    if (!env) return false;
    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) return false;

    MDB_dbi nodes_dbi = 0;
    if (mdb_dbi_open(txn, "nodes", 0, &nodes_dbi) != 0) nodes_dbi = 0;
    if (!nodes_dbi) { mdb_txn_abort(txn); return false; }

    MDB_val k{ sizeof(nodeId), &nodeId };
    MDB_val v{};
    if (mdb_get(txn, nodes_dbi, &k, &v) != 0) { mdb_txn_abort(txn); return false; }

    if (v.mv_size >= (int)sizeof(out)) {
        memcpy(&out, v.mv_data, sizeof(out));
        mdb_txn_abort(txn);
        return true;
    }

    mdb_txn_abort(txn);
    return false;
}

QVector<uint64_t> FSIndexRepository::getChildren(uint64_t parentId)
{
    QVector<uint64_t> out;
    if (!env) return out;
    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) return out;

    MDB_dbi children_dbi = 0;
    if (mdb_dbi_open(txn, "children", 0, &children_dbi) != 0) children_dbi = 0;
    if (!children_dbi) { mdb_txn_abort(txn); return out; }

    MDB_cursor *cursor = nullptr;
    if (mdb_cursor_open(txn, children_dbi, &cursor) != 0) { mdb_txn_abort(txn); return out; }

    MDB_val k{ sizeof(parentId), &parentId };
    MDB_val v{};

    int rc = mdb_cursor_get(cursor, &k, &v, MDB_SET);
    if (rc == 0) {
        do {
            uint64_t child = read_u64_from_val(v);
            out.append(child);
            rc = mdb_cursor_get(cursor, &k, &v, MDB_NEXT_DUP);
        } while (rc == 0);
    }

    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);
    return out;
}


bool FSIndexRepository::getPathForNode(uint64_t nodeId, QString &outPath)
{
    if (!env) return false;
    MDB_txn *txn = nullptr;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) return false;

    MDB_dbi paths_dbi = 0;
    if (mdb_dbi_open(txn, "paths", 0, &paths_dbi) != 0) paths_dbi = 0;
    if (!paths_dbi) { mdb_txn_abort(txn); return false; }

    MDB_cursor *cursor = nullptr;
    if (mdb_cursor_open(txn, paths_dbi, &cursor) != 0) { mdb_txn_abort(txn); return false; }

    MDB_val k{};
    MDB_val v{};
    int rc = mdb_cursor_get(cursor, &k, &v, MDB_FIRST);
    while (rc == 0) {
        uint64_t val = read_u64_from_val(v);
        if (val == nodeId) {
            outPath = QString::fromUtf8(reinterpret_cast<const char*>(k.mv_data), (int)k.mv_size);
            mdb_cursor_close(cursor);
            mdb_txn_abort(txn);
            return true;
        }
        rc = mdb_cursor_get(cursor, &k, &v, MDB_NEXT);
    }

    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);
    return false;
}

