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

bool LMDBManager::initialize(const QString& path)
{
    qInfo() << "[LMDBManager] Initializing Environment";

    int rc = mdb_env_create(&env);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Env Create Failed:" << mdb_strerror(rc);
        return false;
    }

    mdb_env_set_maxdbs(env, 1);
    mdb_env_set_mapsize(env, 1024ULL * 1024ULL * 100ULL); // 100 MB

    QDir dir; //FIXME: Something Broken here
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

bool LMDBManager::writeTestData()
{
    if (!initialized)
    {
        qCritical() << "[LMDBManager] Not Initialized";
        return false;
    }

    qInfo() << "[LMDBManager] Writing Test Data";

    MDB_txn* txn;
    int rc = mdb_txn_begin(env, nullptr, 0, &txn);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Txn Begin Failed:" << mdb_strerror(rc);
        return false;
    }

    MDB_val key, value;

    QByteArray keyData = "case_name";
    QByteArray valueData = "Test Case 001";

    key.mv_size = keyData.size();
    key.mv_data = keyData.data();

    value.mv_size = valueData.size();
    value.mv_data = valueData.data();

    rc = mdb_put(txn, dbi, &key, &value, 0);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Put Failed:" << mdb_strerror(rc);
        mdb_txn_abort(txn);
        return false;
    }

    rc = mdb_txn_commit(txn);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Commit Failed:" << mdb_strerror(rc);
        return false;
    }

    qInfo() << "[LMDBManager] Write Successful";
    return true;
}

bool LMDBManager::readTestData()
{
    if (!initialized)
    {
        qCritical() << "[LMDBManager] Not Initialized";
        return false;
    }

    qInfo() << "[LMDBManager] Reading Test Data";

    MDB_txn* txn;
    int rc = mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Read Txn Begin Failed:" << mdb_strerror(rc);
        return false;
    }

    MDB_val key, value;

    QByteArray keyData = "case_name";
    key.mv_size = keyData.size();
    key.mv_data = keyData.data();

    rc = mdb_get(txn, dbi, &key, &value);
    if (rc != 0)
    {
        qCritical() << "[LMDBManager] Get Failed:" << mdb_strerror(rc);
        mdb_txn_abort(txn);
        return false;
    }

    QByteArray result(
        static_cast<char*>(value.mv_data),
        value.mv_size
        );

    qInfo() << "[LMDBManager] Read Value:" << result;

    mdb_txn_abort(txn);

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
