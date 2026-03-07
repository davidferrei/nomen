#include "lmdbManager.h"

LMDBManager::LMDBManager()
    : env(nullptr),
    dbi(0),
    initialized(false)
{
    qInfo() << "[LMDBManager] Constructor Called";
}

LMDBManager::~LMDBManager()
{
    qInfo() << "[LMDBManager] Destructor Called";
    close();
}

//############################################################################################
// Functions

bool LMDBManager::initialize(const QString& path)
{
    qInfo() << "[LMDBManager] Initializing Environment";

    int rc = mdb_env_create(&env);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Env Create Failed:" << mdb_strerror(rc);
        return false;
    }

    mdb_env_set_maxdbs(env, 16);
    mdb_env_set_mapsize(env, 1024ULL * 1024ULL * 1024ULL * 1ULL); // 1 GB

    QDir dir;
    if (!dir.exists(path))
    {
        if (!dir.mkpath(path))
        {
            qCritical() << "[LMDBManager] Directory Creation Failed";
            return false;
        }
    }

    rc = mdb_env_open(env, path.toUtf8().constData(), 0, 0664);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Env Open Failed:" << mdb_strerror(rc);
        return false;
    }

    MDB_txn* txn;
    rc = mdb_txn_begin(env, nullptr, 0, &txn);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Txn Begin Failed:" << mdb_strerror(rc);
        return false;
    }

    rc = mdb_dbi_open(txn, nullptr, 0, &dbi);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] DBI Open Failed:" << mdb_strerror(rc);
        mdb_txn_abort(txn);
        return false;
    }

    rc = mdb_txn_commit(txn);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Txn Commit Failed:" << mdb_strerror(rc);
        return false;
    }

    initialized = true;

    qInfo() << "[LMDBManager] Initialization Successful";
    return true;
}

void LMDBManager::close()
{
    if (!initialized)
        return;

    qInfo() << "[LMDBManager] Closing Environment";

    mdb_dbi_close(env, dbi);
    mdb_env_close(env);

    env = nullptr;
    initialized = false;
}

//############################################################################################
// Read/Write 

bool LMDBManager::putBytes(const QByteArray& key, const QByteArray& value)
{
    // PerformanceTimer timer("LMDB putBytes");

    if (!initialized) {
        qCritical() << "[LMDBManager]: LMDB not Initialized";
        return false;
    }

    MDB_txn* ltxn = nullptr;
    int rc = mdb_txn_begin(env, nullptr, 0, &ltxn);
    if (rc != 0) {
        qCritical() << "[LMDBManager]: txn Begin Failed:" << mdb_strerror(rc);
        return false;
    }

    MDB_val k{ static_cast<size_t>(key.size()), const_cast<char*>(key.data()) };
    MDB_val v{ static_cast<size_t>(value.size()), const_cast<char*>(value.data()) };

    rc = mdb_put(ltxn, dbi, &k, &v, 0);
    if (rc != 0) {
        qCritical() << "[LMDBManager]: putBytes Failed:" << mdb_strerror(rc);
        mdb_txn_abort(ltxn);
        return false;
    }

    rc = mdb_txn_commit(ltxn);
    if (rc != 0) {
        qCritical() << "[LMDBManager]: putBytes Commit Failed:" << mdb_strerror(rc);
        return false;
    }

    return true;
}

bool LMDBManager::getBytes(const QByteArray& key, QByteArray& outValue)
{
    // PerformanceTimer timer("LMDB getBytes");

    if (!initialized) {
        qCritical() << "[LMDBManager]: LMDB not Initialized";
        return false;
    }

    MDB_txn* ltxn = nullptr;
    int rc = mdb_txn_begin(env, nullptr, MDB_RDONLY, &ltxn);
    if (rc != 0) {
        qCritical() << "[LMDBManager]: getBytes Read txn Begin Failed:" << mdb_strerror(rc);
        return false;
    }

    MDB_val k{ static_cast<size_t>(key.size()), const_cast<char*>(key.data()) };
    MDB_val v{};

    rc = mdb_get(ltxn, dbi, &k, &v);
    if (rc != 0) {
        qCritical() << "[LMDBManager]: getBytes Failed:" << mdb_strerror(rc);
        mdb_txn_abort(ltxn);
        return false;
    }

    outValue = QByteArray(static_cast<char*>(v.mv_data), static_cast<int>(v.mv_size));
    mdb_txn_abort(ltxn);
    return true;
}


//############################################################################################
// Debug

static inline uint64_t read_u64(const MDB_val &v)
{
    if (v.mv_size < (int)sizeof(uint64_t)) return 0;
    uint64_t x = 0;
    memcpy(&x, v.mv_data, sizeof(uint64_t));
    return x;
}

bool LMDBManager::debug_dumpAllLmdbTest(const QString& lmdbPath)
{
    PerformanceTimer timer("Debug Dump All LMDB");

    MDB_env* env = nullptr;
    MDB_txn* txn = nullptr;
    MDB_dbi nodes_dbi = 0, children_dbi = 0, paths_dbi = 0;

    if (mdb_env_create(&env) != 0) {
        qCritical() << "[LMDBManager] Debug: env Create Failed";
        return false;
    }
    mdb_env_set_maxdbs(env, 4);
    mdb_env_set_mapsize(env, 1024ULL * 1024 * 1024 * 10ULL);

    QDir().mkpath(lmdbPath);

    if (mdb_env_open(env, lmdbPath.toUtf8().constData(), 0, 0664) != 0) {
        qCritical() << "[LMDBManager] Debug: env Open Failed for" << lmdbPath;
        mdb_env_close(env);
        return false;
    }

    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) {
        qCritical() << "[LMDBManager] Debug: txn Begin Failed";
        mdb_env_close(env);
        return false;
    }

    if (mdb_dbi_open(txn, "nodes", 0, &nodes_dbi) != 0) nodes_dbi = 0;
    if (mdb_dbi_open(txn, "children", 0, &children_dbi) != 0) children_dbi = 0;
    if (mdb_dbi_open(txn, "paths", 0, &paths_dbi) != 0) paths_dbi = 0;

    MDB_cursor* cursor = nullptr;
    MDB_val k, v;

    uint64_t nodes_count = 0;
    uint64_t children_count = 0;
    uint64_t paths_count = 0;

    if (nodes_dbi) {
        if (mdb_cursor_open(txn, nodes_dbi, &cursor) == 0) {
            while (mdb_cursor_get(cursor, &k, &v, MDB_NEXT) == 0) nodes_count++;
            mdb_cursor_close(cursor);
        }
    }

    if (children_dbi) {
        if (mdb_cursor_open(txn, children_dbi, &cursor) == 0) {
            while (mdb_cursor_get(cursor, &k, &v, MDB_NEXT) == 0) children_count++;
            mdb_cursor_close(cursor);
        }
    }

    if (paths_dbi) {
        if (mdb_cursor_open(txn, paths_dbi, &cursor) == 0) {
            while (mdb_cursor_get(cursor, &k, &v, MDB_NEXT) == 0) paths_count++;
            mdb_cursor_close(cursor);
        }
    }

    qInfo() << "LMDB COUNTS:";
    qInfo() << "  Nodes   :" << nodes_count;
    qInfo() << "  Children:" << children_count;
    qInfo() << "  Paths   :" << paths_count;

    mdb_txn_abort(txn);
    mdb_env_close(env);
    return true;
}

bool LMDBManager::debug_dumpLmdbFirst20(const QString& lmdbPath)
{
    PerformanceTimer timer("Debug Dump First 20 LMDB");

    MDB_env* env = nullptr;
    MDB_txn* txn = nullptr;
    MDB_dbi nodes_dbi = 0, children_dbi = 0, paths_dbi = 0;

    if (mdb_env_create(&env) != 0) {
        qCritical() << "[LMDBManager] Debug: env Create Failed";
        return false;
    }
    mdb_env_set_maxdbs(env, 4);
    mdb_env_set_mapsize(env, 1024ULL * 1024 * 1024 * 10ULL);

    QDir().mkpath(lmdbPath);

    if (mdb_env_open(env, lmdbPath.toUtf8().constData(), 0, 0664) != 0) {
        qCritical() << "[LMDBManager] Debug: env Open Failed for" << lmdbPath;
        mdb_env_close(env);
        return false;
    }

    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) {
        qCritical() << "[LMDBManager] Debug: txn Begin Failed";
        mdb_env_close(env);
        return false;
    }

    if (mdb_dbi_open(txn, "nodes", 0, &nodes_dbi) != 0) nodes_dbi = 0;
    if (mdb_dbi_open(txn, "children", 0, &children_dbi) != 0) children_dbi = 0;
    if (mdb_dbi_open(txn, "paths", 0, &paths_dbi) != 0) paths_dbi = 0;

    MDB_cursor* cursor = nullptr;
    MDB_val k, v;
    int count = 0;

    qInfo() << "---- NODES (first 20) ----";
    count = 0;
    if (nodes_dbi && mdb_cursor_open(txn, nodes_dbi, &cursor) == 0) {
        while (count < 20 && mdb_cursor_get(cursor, &k, &v, MDB_NEXT) == 0) {
            uint64_t id = read_u64(k);

            struct NodeMetadata {
                uint64_t inode;
                uint64_t size;
                uint64_t ctime;
                uint64_t mtime;
                uint8_t type;
                uint8_t is_dir;
            } meta{};

            if (v.mv_size >= (int)sizeof(meta)) {
                memcpy(&meta, v.mv_data, sizeof(meta));
                qInfo() << "id:" << id
                        << "inode:" << meta.inode
                        << "size:" << meta.size
                        << "ctime:" << meta.ctime
                        << "mtime:" << meta.mtime
                        << "type:" << (int)meta.type
                        << "is_dir:" << (int)meta.is_dir;
            } else {
                qInfo() << "id:" << id << "(raw size:" << (int)v.mv_size << ")";
            }
            count++;
        }
        mdb_cursor_close(cursor);
    }

    qInfo() << "---- CHILDREN (first 20) ----";
    count = 0;
    if (children_dbi && mdb_cursor_open(txn, children_dbi, &cursor) == 0) {
        while (count < 20 && mdb_cursor_get(cursor, &k, &v, MDB_NEXT) == 0) {
            uint64_t pk = read_u64(k);
            uint64_t pv = read_u64(v);
            qInfo() << pk << "->" << pv;
            count++;
        }
        mdb_cursor_close(cursor);
    }

    qInfo() << "---- PATHS (first 20) ----";
    count = 0;
    if (paths_dbi && mdb_cursor_open(txn, paths_dbi, &cursor) == 0) {
        while (count < 20 && mdb_cursor_get(cursor, &k, &v, MDB_NEXT) == 0) {
            QString keyStr = QString::fromUtf8(static_cast<char*>(k.mv_data), (int)k.mv_size);
            uint64_t mapped = read_u64(v);
            qInfo() << keyStr << "=>" << mapped;
            count++;
        }
        mdb_cursor_close(cursor);
    }

    mdb_txn_abort(txn);
    mdb_env_close(env);
    return true;
}
