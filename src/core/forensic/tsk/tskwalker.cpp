#include "tskwalker.h"

struct WalkContext {
    MDB_txn* txn;
    MDB_dbi nodes_dbi;
    MDB_dbi children_dbi;
    MDB_dbi paths_dbi;
    MDB_dbi inode_map_dbi;
    uint64_t next_node_id;
    uint64_t file_count;
    uint64_t dir_count;
    int partitionId;
    MDB_env* env;
    uint64_t commit_batch;
};

static TSK_WALK_RET_ENUM file_callback(
    TSK_FS_FILE* fs_file,
    const char* path,
    void* ptr)
{
    if (!fs_file || !fs_file->name || !fs_file->name->name)
        return TSK_WALK_CONT;

    const char* rawName = fs_file->name->name;
    if (rawName[0] == '.' && (rawName[1] == '\0' || (rawName[1] == '.' && rawName[2] == '\0')))
        return TSK_WALK_CONT;

    auto* ctx = static_cast<WalkContext*>(ptr);
    if (!ctx || !ctx->txn) return TSK_WALK_CONT;

    bool is_dir = (fs_file->meta && fs_file->meta->type == TSK_FS_META_TYPE_DIR);
    if (is_dir) ctx->dir_count++; else ctx->file_count++;

    QString parentPath = QString::fromUtf8(path);
    if (parentPath.isEmpty()) parentPath = "/";
    if (!parentPath.startsWith('/')) parentPath.prepend('/');
    while (parentPath.size() > 1 && parentPath.endsWith('/')) parentPath.chop(1);

    QString name = QString::fromUtf8(rawName);
    QString fullPath = (parentPath == "/") ? ("/" + name) : (parentPath + "/" + name);

    QByteArray fullUtf8 = fullPath.toUtf8();

    uint64_t parent_id = 0;
    if (fs_file->name->par_addr != 0) {
        MDB_val pk{ sizeof(fs_file->name->par_addr), &fs_file->name->par_addr };
        MDB_val pv{};
        if (mdb_get(ctx->txn, ctx->inode_map_dbi, &pk, &pv) == 0)
            parent_id = *static_cast<uint64_t*>(pv.mv_data);
    }

    if (parent_id == 0)
        return TSK_WALK_CONT;

    uint64_t node_id = ctx->next_node_id++;
    uint64_t inode = fs_file->name->meta_addr;

    MDB_val ik{ sizeof(inode), &inode };
    MDB_val iv{ sizeof(node_id), &node_id };
    mdb_put(ctx->txn, ctx->inode_map_dbi, &ik, &iv, 0);

    struct NodeMetadata {
        uint64_t inode;
        uint64_t size;
        uint64_t ctime;
        uint64_t mtime;
        uint8_t type;
        uint8_t is_dir;
    } meta{};

    meta.inode = fs_file->name->meta_addr;
    meta.size  = fs_file->meta ? fs_file->meta->size : 0;
    meta.type  = fs_file->meta ? fs_file->meta->type : 0;
    meta.ctime = fs_file->meta ? fs_file->meta->crtime : 0;
    meta.mtime = fs_file->meta ? fs_file->meta->mtime : 0;
    meta.is_dir = is_dir ? 1 : 0;

    MDB_val nk{ sizeof(node_id), &node_id };
    MDB_val nv{ sizeof(meta), &meta };
    mdb_put(ctx->txn, ctx->nodes_dbi, &nk, &nv, 0);

    MDB_val pkpath{ static_cast<size_t>(fullUtf8.size()), const_cast<char*>(fullUtf8.data()) };
    MDB_val pvpath{ sizeof(node_id), &node_id };
    mdb_put(ctx->txn, ctx->paths_dbi, &pkpath, &pvpath, 0);

    if (parent_id != 0) {
        MDB_val ck{ sizeof(parent_id), &parent_id };
        MDB_val cv{ sizeof(node_id), &node_id };
        mdb_put(ctx->txn, ctx->children_dbi, &ck, &cv, 0);
    }

        if (ctx->commit_batch > 0 && (ctx->next_node_id % ctx->commit_batch) == 0) {
                int rc = mdb_txn_commit(ctx->txn);
                if (rc != 0) {
                    qWarning() << "[TskWalker]: Commit Failed During Walk" << node_id << ":" << mdb_strerror(rc);
                    mdb_txn_abort(ctx->txn);
                    return TSK_WALK_STOP;
            }
            MDB_txn* newtxn = nullptr;
                int rc2 = mdb_txn_begin(ctx->env, nullptr, 0, &newtxn);
                if (rc2 != 0) {
                    qWarning() << "[TskWalker]: Failed to Begin New txn After Commit:" << mdb_strerror(rc2);
                    return TSK_WALK_STOP;
            }
            ctx->txn = newtxn;
        }

        return TSK_WALK_CONT;
}

bool TskWalker::walkImage(const QString &imagePath, LMDBManager &lmdb)
{
    qInfo() << "[TskWalker] Opening Image:" << imagePath;

    TSK_IMG_INFO *img = tsk_img_open_sing(imagePath.toUtf8().data(), TSK_IMG_TYPE_DETECT, 0);
    if (!img) {
        qWarning() << "[TskWalker] Failed to Open Image";
        return false;
    }

    TSK_FS_INFO *fs = nullptr;
    TSK_VS_INFO *vs = tsk_vs_open(img, 0, TSK_VS_TYPE_DETECT);
    if (vs) {
        for (TSK_PNUM_T i = 0; i < vs->part_count; i++) {
            const TSK_VS_PART_INFO *part = tsk_vs_part_get(vs, i);
            if (!part) continue;
            fs = tsk_fs_open_vol(part, TSK_FS_TYPE_DETECT);
            if (fs) break;
        }
        tsk_vs_close(vs);
    } else {
        fs = tsk_fs_open_img(img, 0, TSK_FS_TYPE_DETECT);
    }

    if (!fs) {
        qWarning() << "[TskWalker] Failed to Open Filesystem";
        tsk_img_close(img);
        return false;
    }

    MDB_env* env = lmdb.envHandle();
    if (!env) {
        qWarning() << "[TskWalker] LMDB environment not available";
        tsk_fs_close(fs);
        tsk_img_close(img);
        return false;
    }

    MDB_txn* txn = nullptr;
    if (mdb_txn_begin(env, nullptr, 0, &txn) != 0) {
        qWarning() << "[TskWalker] Failed to begin LMDB txn";
        tsk_fs_close(fs);
        tsk_img_close(img);
        return false;
    }

    MDB_dbi nodes_dbi, children_dbi, paths_dbi, inode_map_dbi;
    if (mdb_dbi_open(txn, "nodes", MDB_CREATE, &nodes_dbi) != 0) nodes_dbi = 0;
    if (mdb_dbi_open(txn, "children", MDB_CREATE | MDB_DUPSORT, &children_dbi) != 0) children_dbi = 0;
    if (mdb_dbi_open(txn, "paths", MDB_CREATE, &paths_dbi) != 0) paths_dbi = 0;
    if (mdb_dbi_open(txn, "inode_map", MDB_CREATE, &inode_map_dbi) != 0) inode_map_dbi = 0;

    WalkContext ctx{};
    ctx.txn = txn;
    ctx.nodes_dbi = nodes_dbi;
    ctx.children_dbi = children_dbi;
    ctx.paths_dbi = paths_dbi;
    ctx.inode_map_dbi = inode_map_dbi;
    ctx.next_node_id = 1;
    ctx.file_count = 0;
    ctx.dir_count = 0;
    ctx.partitionId = 0;
    ctx.env = env;
    ctx.commit_batch = 10000; 

    // Root node
    uint64_t root_id = ctx.next_node_id++;
    struct RootMeta { uint8_t is_dir; } root{};
    root.is_dir = 1;
    MDB_val rk{ sizeof(root_id), &root_id };
    MDB_val rv{ sizeof(root), &root };
    if (nodes_dbi) mdb_put(txn, nodes_dbi, &rk, &rv, 0);

    if (paths_dbi) {
        MDB_val pk{ 1, const_cast<char*>((char*)"/") };
        MDB_val pv{ sizeof(root_id), &root_id };
        mdb_put(txn, paths_dbi, &pk, &pv, 0);
    }

    if (inode_map_dbi) {
        uint64_t root_inode = fs->root_inum;
        MDB_val ik{ sizeof(root_inode), &root_inode };
        MDB_val iv{ sizeof(root_id), &root_id };
        mdb_put(txn, inode_map_dbi, &ik, &iv, 0);
    }

    PerformanceTimer pt(QString("TSK walkImage %1").arg(imagePath));

    tsk_fs_dir_walk(fs, fs->root_inum, TSK_FS_DIR_WALK_FLAG_RECURSE, file_callback, &ctx);

    if (ctx.txn) {
        int rc = mdb_txn_commit(ctx.txn);
        if (rc != 0) {
            qWarning() << "[TskWalker] Failed to Commit LMDB txn:" << mdb_strerror(rc);
            mdb_txn_abort(ctx.txn);
        }
    }

    if (nodes_dbi) mdb_dbi_close(env, nodes_dbi);
    if (children_dbi) mdb_dbi_close(env, children_dbi);
    if (paths_dbi) mdb_dbi_close(env, paths_dbi);
    if (inode_map_dbi) mdb_dbi_close(env, inode_map_dbi);

    tsk_fs_close(fs);
    tsk_img_close(img);

    qInfo() << "Disk walk completed:";
    qInfo() << "Directories:" << ctx.dir_count;
    qInfo() << "Files:" << ctx.file_count;
    qInfo() << "Total nodes:" << (ctx.file_count + ctx.dir_count + 1);

    return true;
}
