#include "tskwalker.h"


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
    if (!ctx || !ctx->lmdb_txn) return TSK_WALK_CONT;

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
    uint64_t par_addr = 0;
    if (fs_file->name->par_addr != 0) {
        unsigned char ikbuf[24];
        uint64_t part = (uint64_t)ctx->partitionId;
        uint64_t fsIdx = 0;
        par_addr = (uint64_t)fs_file->name->par_addr;
        memcpy(ikbuf, &part, sizeof(part));
        memcpy(ikbuf + sizeof(part), &fsIdx, sizeof(fsIdx));
        memcpy(ikbuf + sizeof(part) + sizeof(fsIdx), &par_addr, sizeof(par_addr));
        MDB_val pk{ sizeof(ikbuf), ikbuf };
        MDB_val pv{};
        if (mdb_get(ctx->lmdb_txn, ctx->lmdb_inode_map_dbi, &pk, &pv) == 0)
            parent_id = *static_cast<uint64_t*>(pv.mv_data);
    }

    if (parent_id == 0) {
        qWarning() << "[TskWalker] Parent Lookup Failed" << "part:" << ctx->partitionId << "par_addr:" << par_addr << "path:" << fullPath;
        return TSK_WALK_CONT;
    }

    uint64_t node_id = ctx->next_node_id++;
    uint64_t inode = fs_file->name->meta_addr;

    unsigned char ikbuf2[24];
    uint64_t part = (uint64_t)ctx->partitionId;
    uint64_t fsIdx = 0;
    memcpy(ikbuf2, &part, sizeof(part));
    memcpy(ikbuf2 + sizeof(part), &fsIdx, sizeof(fsIdx));
    memcpy(ikbuf2 + sizeof(part) + sizeof(fsIdx), &inode, sizeof(inode));
    MDB_val ik{ sizeof(ikbuf2), ikbuf2 };
    MDB_val iv{ sizeof(node_id), &node_id };
    mdb_put(ctx->lmdb_txn, ctx->lmdb_inode_map_dbi, &ik, &iv, MDB_NOOVERWRITE);

    NodeMetadata meta{};

    meta.inode = fs_file->name->meta_addr;
    meta.size  = fs_file->meta ? fs_file->meta->size : 0;
    meta.type  = fs_file->meta ? fs_file->meta->type : 0;
    meta.ctime = fs_file->meta ? fs_file->meta->crtime : 0;
    meta.mtime = fs_file->meta ? fs_file->meta->mtime : 0;
    meta.is_dir = is_dir ? 1 : 0;

    MDB_val nk{ sizeof(node_id), &node_id };
    MDB_val nv{ sizeof(meta), &meta };
    mdb_put(ctx->lmdb_txn, ctx->lmdb_nodes_dbi, &nk, &nv, 0);

    QString namespaced = QString("%1:%2:%3").arg((uint64_t)part).arg((uint64_t)0).arg(fullPath);
    QByteArray nsUtf8 = namespaced.toUtf8();
    MDB_val pkpath{ static_cast<size_t>(nsUtf8.size()), const_cast<char*>(nsUtf8.data()) };
    MDB_val pvpath{ sizeof(node_id), &node_id };
    mdb_put(ctx->lmdb_txn, ctx->lmdb_paths_dbi, &pkpath, &pvpath, 0);

    if (parent_id != 0) {
        MDB_val ck{ sizeof(parent_id), &parent_id };
        MDB_val cv{ sizeof(node_id), &node_id };
        mdb_put(ctx->lmdb_txn, ctx->lmdb_children_dbi, &ck, &cv, 0);
    }

    // Debug
    // if ((ctx->next_node_id % 1000) == 0) {
    //     qInfo() << "[TskWalker] Processed Nodes:" << ctx->next_node_id << "Files:" << ctx->file_count << "Dirs:" << ctx->dir_count;
    // }

    if (ctx->lmdb_commit_batch > 0 && (ctx->next_node_id % ctx->lmdb_commit_batch) == 0) {
                int rc = mdb_txn_commit(ctx->lmdb_txn);
                if (rc != 0) {
                    qWarning() << "[TskWalker]: Commit Failed During Walk" << node_id << ":" << mdb_strerror(rc);
                    mdb_txn_abort(ctx->lmdb_txn);
                    return TSK_WALK_STOP;
            }
            MDB_txn* newtxn = nullptr;
                int rc2 = mdb_txn_begin(ctx->lmdb_env, nullptr, 0, &newtxn);
                if (rc2 != 0) {
                    qWarning() << "[TskWalker]: Failed to Begin New txn After Commit:" << mdb_strerror(rc2);
                    return TSK_WALK_STOP;
            }
            ctx->lmdb_txn = newtxn;
        }

        return TSK_WALK_CONT;
}


//############################################################################################
// Helper Functions

static bool setup_lmdb_connection(LMDBManager &lmdb, TSK_IMG_INFO* img, MDB_env*& env, MDB_txn*& txn,
                                  MDB_dbi &nodes_dbi, MDB_dbi &children_dbi, MDB_dbi &paths_dbi, MDB_dbi &inode_map_dbi)
{
    env = lmdb.envHandle();
    if (!env) 
    {
        qWarning() << "[TskWalker] LMDB Environment not Available";

        tsk_img_close(img);

        return false;
    }

    txn = nullptr;
    if (mdb_txn_begin(env, nullptr, 0, &txn) != 0) 
    {
        qWarning() << "[TskWalker] Failed to Begin LMDB txn";

        tsk_img_close(img);

        return false;
    }

    qInfo() << "[TskWalker] LMDB Write txn Begin";

    nodes_dbi = 0; children_dbi = 0; paths_dbi = 0; inode_map_dbi = 0;
    int rc = 0;

    rc = mdb_dbi_open(txn, "nodes", MDB_CREATE, &nodes_dbi);
    if (rc != 0) 
    {
        qWarning() << "[TskWalker] Failed to open LMDB 'nodes' DB:" << mdb_strerror(rc);
        mdb_txn_abort(txn);
        tsk_img_close(img);
        return false;
    }

    rc = mdb_dbi_open(txn, "children", MDB_CREATE | MDB_DUPSORT, &children_dbi);
    if (rc != 0) 
    {
        qWarning() << "[TskWalker] Failed to open LMDB 'children' DB:" << mdb_strerror(rc);
        mdb_txn_abort(txn);
        tsk_img_close(img);
        return false;
    }

    rc = mdb_dbi_open(txn, "paths", MDB_CREATE, &paths_dbi);
    if (rc != 0) 
    {
        qWarning() << "[TskWalker] Failed to open LMDB 'paths' DB:" << mdb_strerror(rc);
        mdb_txn_abort(txn);
        tsk_img_close(img);
        return false;
    }

    rc = mdb_dbi_open(txn, "inode_map", MDB_CREATE, &inode_map_dbi);
    if (rc != 0) 
    {
        qWarning() << "[TskWalker] Failed to open LMDB 'inode_map' DB:" << mdb_strerror(rc);
        mdb_txn_abort(txn);
        tsk_img_close(img);
        return false;
    }

    return true;
}



//############################################################################################
// TSK walker

bool TskWalker::walkImage(const QString &imagePath, LMDBManager &lmdb)
{
    qInfo() << "[TskWalker] Opening Image:" << imagePath;

    TSK_IMG_INFO *img = tsk_img_open_sing(imagePath.toUtf8().data(), TSK_IMG_TYPE_DETECT, 0);
    if (!img) {
        qWarning() << "[TskWalker] Failed to Open Image" << imagePath << ":" << tsk_error_get();
        return false;
    }

    qInfo() << "[TskWalker] Image Opened Successfully";


    TSK_FS_INFO *fs = nullptr;
    TSK_VS_INFO *vs = tsk_vs_open(img, 0, TSK_VS_TYPE_DETECT);

    qInfo() << "[TskWalker] Volume System Probe Returned" << (vs ? "Present" : "None");

    std::vector<std::pair<TSK_FS_INFO*, int>> fslist;

    if (vs) 
    {
        for (TSK_PNUM_T i = 0; i < vs->part_count; i++) 
        {
            const TSK_VS_PART_INFO *part = tsk_vs_part_get(vs, i);

            if (!part) continue;
            
            TSK_FS_INFO* pfs = tsk_fs_open_vol(part, TSK_FS_TYPE_DETECT);

            qInfo() << "[TskWalker] Probing Partition" << i << "FS=" << (pfs ? "OK" : "No");

            if (pfs) fslist.emplace_back(pfs, (int)i);
        }

        tsk_vs_close(vs);
    } 
    else 
    {
        TSK_FS_INFO* pfs = tsk_fs_open_img(img, 0, TSK_FS_TYPE_DETECT);

        qInfo() << "[TskWalker] Probing Image-Level Filesystem, Result=" << (pfs ? "OK" : "No");

        if (pfs) fslist.emplace_back(pfs, 0);
    }

    if (fslist.empty()) 
    {
        qWarning() << "[TskWalker] Failed to Find Filesystems in:" << imagePath;

        tsk_img_close(img);

        return false;
    }

    MDB_env* env = nullptr;
    MDB_txn* txn = nullptr;
    MDB_dbi nodes_dbi = 0, children_dbi = 0, paths_dbi = 0, inode_map_dbi = 0;

    if (!setup_lmdb_connection(lmdb, img, env, txn, nodes_dbi, children_dbi, paths_dbi, inode_map_dbi)) 
    {
        qWarning() << "[TskWalker] Failed to Setup LMDB Connection";
        return false;
    }

    WalkContext walk_ctx{};
    walk_ctx.lmdb_txn = txn;
    walk_ctx.lmdb_env = env;
    walk_ctx.lmdb_commit_batch = 10000;

    walk_ctx.lmdb_nodes_dbi = nodes_dbi;
    walk_ctx.lmdb_children_dbi = children_dbi;
    walk_ctx.lmdb_paths_dbi = paths_dbi;
    walk_ctx.lmdb_inode_map_dbi = inode_map_dbi;
    walk_ctx.next_node_id = 1;
    walk_ctx.file_count = 0;
    walk_ctx.dir_count = 0;
    walk_ctx.partitionId = 0;

    PerformanceTimer pt("Total Disk Walk Time (" + imagePath + ")");

    qInfo() << "[TskWalker] Starting Directory Walk ...";

    // Defining a Syntetic Root Node for the DS
    uint64_t top_root_id = walk_ctx.next_node_id++;
    NodeMetadata diskRootNode{};
    diskRootNode.is_dir = 1;
    diskRootNode.inode = 0;
    diskRootNode.size = 0;
    diskRootNode.type = 0;
    diskRootNode.ctime = 0;
    diskRootNode.mtime = 0;

    MDB_val trk{ sizeof(top_root_id), &top_root_id };
    MDB_val trv{ sizeof(diskRootNode), &diskRootNode };
    mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_nodes_dbi, &trk, &trv, 0);

    QByteArray slash = QByteArray("/");
    MDB_val pk{ static_cast<size_t>(slash.size()), const_cast<char*>(slash.data()) };
    MDB_val pv{ sizeof(top_root_id), &top_root_id };
    mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_paths_dbi, &pk, &pv, 0);


    // Iterate Filesystems
    for (auto& [fsInfo, partitionID] : fslist) 
    {
        walk_ctx.partitionId = partitionID;

        uint64_t part_root_id = walk_ctx.next_node_id++;
        NodeMetadata fsRoot{};
        fsRoot.is_dir = 1;
        fsRoot.inode = fsInfo->root_inum;
        fsRoot.size = 0;
        fsRoot.type = 0;
        fsRoot.ctime = 0;
        fsRoot.mtime = 0;
        MDB_val rk{ sizeof(part_root_id), &part_root_id };
        MDB_val rv{ sizeof(fsRoot), &fsRoot };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_nodes_dbi, &rk, &rv, 0);

        uint64_t fsIdx = 0; //FIXME: Handle Multiple FS per Partition
        QString partPath = QString("%1:%2:/").arg((uint64_t)partitionID).arg((uint64_t)fsIdx);
        QByteArray partUtf8 = partPath.toUtf8();


        if (paths_dbi) {
            MDB_val pk{ static_cast<size_t>(partUtf8.size()), const_cast<char*>(partUtf8.data()) };
            MDB_val pv{ sizeof(part_root_id), &part_root_id };
            mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_paths_dbi, &pk, &pv, 0);
        }

        if (inode_map_dbi) {
            unsigned char ikbuf[24];
            uint64_t part = (uint64_t)partitionID;
            uint64_t root_inode = fsInfo->root_inum;
            uint64_t fsIdxLocal = fsIdx;
            memcpy(ikbuf, &part, sizeof(part));
            memcpy(ikbuf + sizeof(part), &fsIdxLocal, sizeof(fsIdxLocal));
            memcpy(ikbuf + sizeof(part) + sizeof(fsIdxLocal), &root_inode, sizeof(root_inode));
            MDB_val ik{ sizeof(ikbuf), ikbuf };
            MDB_val iv{ sizeof(part_root_id), &part_root_id };
            mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_inode_map_dbi, &ik, &iv, 0);
        }

        MDB_val ck{ sizeof(top_root_id), &top_root_id };
        MDB_val cv{ sizeof(part_root_id), &part_root_id };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_children_dbi, &ck, &cv, 0);

        tsk_fs_dir_walk(fsInfo, fsInfo->root_inum, TSK_FS_DIR_WALK_FLAG_RECURSE, file_callback, &walk_ctx);
        qInfo() << "[TskWalker] Directory Walk Returned for partition" << partitionID;

        tsk_fs_close(fsInfo);
    }

    if (walk_ctx.lmdb_txn) {
        int rc = mdb_txn_commit(walk_ctx.lmdb_txn);
        if (rc != 0) {
            qWarning() << "[TskWalker] Failed to Commit LMDB txn:" << mdb_strerror(rc);
            mdb_txn_abort(walk_ctx.lmdb_txn);
        }
    }

    if (nodes_dbi) mdb_dbi_close(env, nodes_dbi);
    if (children_dbi) mdb_dbi_close(env, children_dbi);
    if (paths_dbi) mdb_dbi_close(env, paths_dbi);
    if (inode_map_dbi) mdb_dbi_close(env, inode_map_dbi);

    tsk_fs_close(fs);
    tsk_img_close(img);

    qInfo() << "[TskWalker] Disk Walk Completed:";
    qInfo() << "[TskWalker] Directories:" << walk_ctx.dir_count;
    qInfo() << "[TskWalker] Files:" << walk_ctx.file_count;
    qInfo() << "[TskWalker] Total Nodes:" << (walk_ctx.file_count + walk_ctx.dir_count + 1);

    return true;
}

