#include "tskwalker.h"

static QString normalizeParentPath(const char* path)
{
    if (!path || path[0] == '\0')
        return QStringLiteral("/");

    size_t len = std::strlen(path);
    while (len > 1 && path[len - 1] == '/') --len;

    if (path[0] == '/') 
    {
        return QString::fromUtf8(path, static_cast<int>(len));
    } 
    else 
    {
        QString out;
        out.reserve(static_cast<int>(len + 1));
        out.append('/');
        out.append(QString::fromUtf8(path, static_cast<int>(len)));
        return out;
    }
}

static QByteArray normalizeParentPathBytes(const char* path)
{
    if (!path || path[0] == '\0') return QByteArray("/");

    size_t len = std::strlen(path);
    while (len > 1 && path[len - 1] == '/') --len;

    if (path[0] == '/') return QByteArray(path, static_cast<int>(len));

    QByteArray out;
    out.reserve(static_cast<int>(len + 1));
    out.append('/');
    out.append(path, static_cast<int>(len));
    return out;
}

static TSK_WALK_RET_ENUM file_callback(TSK_FS_FILE* fs_file, const char* path, void* ptr)
{
    if (!fs_file || !fs_file->name || !fs_file->name->name)
        return TSK_WALK_CONT;

    const char* rawName = fs_file->name->name;
    if (rawName[0] == '.' && (rawName[1] == '\0' || (rawName[1] == '.' && rawName[2] == '\0')))
        return TSK_WALK_CONT;

    auto* walk_ctx = static_cast<WalkContext*>(ptr);
    if (!walk_ctx || !walk_ctx->lmdb_txn) return TSK_WALK_CONT;

    TSK_FS_META *fs_meta = fs_file->meta;

    bool is_dir = (fs_meta && fs_meta->type == TSK_FS_META_TYPE_DIR);
    is_dir ? ++walk_ctx->dir_count : ++walk_ctx->file_count;

    QByteArray parentByteArray = normalizeParentPathBytes(path);
    QByteArray nameByteArray = QByteArray::fromRawData(rawName, ((int)std::strlen(rawName)));

    uint64_t parent_id = 0;
    uint64_t par_addr = 0;

    // 0=allocated, 1=unallocated (meta present), 2=deleted (meta lost), 3=orphan (set below)
    int status = 0;
    if (fs_file->name->flags & TSK_FS_NAME_FLAG_UNALLOC)
        status = (fs_meta != nullptr) ? 1 : 2;

    if (fs_file->name->par_addr != 0)
    {
        unsigned char parentNodeArray[24];
        uint64_t part = (uint64_t)walk_ctx->partitionId;
        uint64_t fsIdx = (uint64_t)walk_ctx->fsIndex;
        par_addr = (uint64_t)fs_file->name->par_addr;
        memcpy(parentNodeArray, &part, sizeof(part));
        memcpy(parentNodeArray + sizeof(part), &fsIdx, sizeof(fsIdx));
        memcpy(parentNodeArray + sizeof(part) + sizeof(fsIdx), &par_addr, sizeof(par_addr));
        MDB_val pk{ sizeof(parentNodeArray), parentNodeArray };
        MDB_val pv{};
        if (mdb_get(walk_ctx->lmdb_txn, walk_ctx->lmdb_inode_map_dbi, &pk, &pv) == 0)
            parent_id = *static_cast<uint64_t*>(pv.mv_data);
    }


    if (parent_id == 0)
    {
        status = (status == 0) ? 3 : status;

        const uint64_t key = ((uint64_t)walk_ctx->partitionId << 32) | (uint64_t)walk_ctx->fsIndex;
        std::unordered_map<uint64_t, uint64_t>* containerMap = nullptr;

        switch (status)
        {
        case 1: 
            containerMap = &walk_ctx->unalloc_node_ids; 
            break;
        case 2: 
            containerMap = &walk_ctx->deleted_node_ids; 
            break;
        case 3: 
            containerMap = &walk_ctx->orphan_node_ids;  
            break;
        }

        auto it = containerMap->find(key);
        if (it == containerMap->end())
        {
            qWarning() << "[TskWalker] Parent Node missing in Partition:" << walk_ctx->partitionId << "FS:" << walk_ctx->fsIndex << "Status:" << status;
            return TSK_WALK_CONT;
        }
        parent_id = it->second;
    }

    uint64_t node_id = walk_ctx->next_node_id++;
    int rc = 0;

    unsigned char iNodeArray[24];
    uint64_t part = (uint64_t)walk_ctx->partitionId;
    uint64_t fsIdx = (uint64_t)walk_ctx->fsIndex;
    memcpy(iNodeArray, &part, sizeof(part));
    memcpy(iNodeArray + sizeof(part), &fsIdx, sizeof(fsIdx));
    memcpy(iNodeArray + sizeof(part) + sizeof(fsIdx), &fs_file->name->meta_addr, sizeof(fs_file->name->meta_addr));
    MDB_val ik{ sizeof(iNodeArray), iNodeArray };
    MDB_val iv{ sizeof(node_id), &node_id };
    rc = mdb_put(walk_ctx->lmdb_txn, walk_ctx->lmdb_inode_map_dbi, &ik, &iv, MDB_NOOVERWRITE);
    if (rc != 0 && rc != MDB_KEYEXIST)
    {
        qWarning() << "[TskWalker] mdb_put inode_map failed:" << mdb_strerror(rc);
        mdb_txn_abort(walk_ctx->lmdb_txn);
        walk_ctx->lmdb_txn = nullptr;
        return TSK_WALK_STOP;
    }

    NodeMetadata nodeMetadata{};
    nodeMetadata.inode = fs_file->name->meta_addr;
    nodeMetadata.size  = fs_meta ? fs_meta->size : 0;
    nodeMetadata.type  = fs_meta ? fs_meta->type : 0;
    nodeMetadata.ctime = fs_meta ? fs_meta->crtime : 0;
    nodeMetadata.mtime = fs_meta ? fs_meta->mtime : 0;
    nodeMetadata.is_dir = (fs_meta && fs_meta->type == TSK_FS_META_TYPE_DIR) ? 1 : 0;
    nodeMetadata.status = (uint8_t)status;

    MDB_val nk{ sizeof(node_id), &node_id };
    MDB_val nv{ sizeof(nodeMetadata), &nodeMetadata };
    rc = mdb_put(walk_ctx->lmdb_txn, walk_ctx->lmdb_nodes_dbi, &nk, &nv, 0);
    if (rc != 0) 
    {
        qWarning() << "[TskWalker] mdb_put nodes failed:" << mdb_strerror(rc);
        mdb_txn_abort(walk_ctx->lmdb_txn);
        walk_ctx->lmdb_txn = nullptr;
        return TSK_WALK_STOP;
    }

    walk_ctx->tmp_key.clear();
    walk_ctx->tmp_key.append(QByteArray::number((qulonglong)part));
    walk_ctx->tmp_key.append(':');
    walk_ctx->tmp_key.append(QByteArray::number((qulonglong)fsIdx));
    walk_ctx->tmp_key.append(':');
    if (parentByteArray == "/") 
    {
        walk_ctx->tmp_key.append('/');
        walk_ctx->tmp_key.append(nameByteArray);
    } else 
    {
        walk_ctx->tmp_key.append(parentByteArray);
        walk_ctx->tmp_key.append('/');
        walk_ctx->tmp_key.append(nameByteArray);
    }

    MDB_val pkpath{ static_cast<size_t>(walk_ctx->tmp_key.size()), const_cast<char*>(walk_ctx->tmp_key.data()) };
    MDB_val pvpath{ sizeof(node_id), &node_id };
    rc = mdb_put(walk_ctx->lmdb_txn, walk_ctx->lmdb_paths_dbi, &pkpath, &pvpath, 0);
    if (rc != 0) 
    {
        qWarning() << "[TskWalker] mdb_put paths failed:" << mdb_strerror(rc);
        mdb_txn_abort(walk_ctx->lmdb_txn);
        walk_ctx->lmdb_txn = nullptr;
        return TSK_WALK_STOP;
    }

    MDB_val ck{ sizeof(parent_id), &parent_id };
    MDB_val cv{ sizeof(node_id), &node_id };
    rc = mdb_put(walk_ctx->lmdb_txn, walk_ctx->lmdb_children_dbi, &ck, &cv, 0);
    if (rc != 0) 
    {
        qWarning() << "[TskWalker] mdb_put children failed:" << mdb_strerror(rc);
        mdb_txn_abort(walk_ctx->lmdb_txn);
        walk_ctx->lmdb_txn = nullptr;
        return TSK_WALK_STOP;
    }

    if (node_id % walk_ctx->lmdb_commit_batch == 0) 
    {
        rc = mdb_txn_commit(walk_ctx->lmdb_txn);

        if (rc != 0) 
        {
            qWarning() << "[TskWalker]: Commit Failed During Walk" << node_id << ":" << mdb_strerror(rc);
            mdb_txn_abort(walk_ctx->lmdb_txn);
            walk_ctx->lmdb_txn = nullptr;
            return TSK_WALK_STOP;
        }

        MDB_txn* newtxn = nullptr;
        rc = mdb_txn_begin(walk_ctx->lmdb_env, nullptr, 0, &newtxn);
        if (rc != 0) {
            qWarning() << "[TskWalker]: Failed to Begin New txn After Commit:" << mdb_strerror(rc);
            walk_ctx->lmdb_txn = nullptr;
            return TSK_WALK_STOP;
        }

        walk_ctx->lmdb_txn = newtxn;
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
    if (!img) 
    {
        qWarning() << "[TskWalker] Failed to Open Image" << imagePath << ":" << tsk_error_get();
        return false;
    }

    qInfo() << "[TskWalker] Image Opened Successfully";


    TSK_VS_INFO *vs = tsk_vs_open(img, 0, TSK_VS_TYPE_DETECT);

    qInfo() << "[TskWalker] Volume System Probe Returned" << (vs ? "Present" : "None");

    std::vector<std::tuple<TSK_FS_INFO*, int, int>> fsVector;
                     //[fsInfo, partitionID, fsIndex]

    std::unordered_map<int,int> fsIndex;

    struct RawPartitionInfo { TSK_PNUM_T addr; uint64_t sizeBytes; std::string desc; };
    std::vector<RawPartitionInfo> rawPartitions;

    if (vs) 
    {
        for (TSK_PNUM_T i = 0; i < vs->part_count; i++) 
        {
            const TSK_VS_PART_INFO *part = tsk_vs_part_get(vs, i);

            if (!part) continue;
            
            TSK_FS_INFO* pfs = tsk_fs_open_vol(part, TSK_FS_TYPE_DETECT);

            qInfo() << "[TskWalker] Probing Partition" << i << "FS=" << (pfs ? "OK" : "No");

            if (pfs)
            {
                int partitionID = (int)i;
                fsVector.emplace_back(pfs, partitionID, fsIndex[partitionID]++);
            }
            else
            {
                rawPartitions.push_back({ part->addr, (uint64_t)part->len * vs->img_info->sector_size,
                                          (part->desc ? part->desc : "") });
            }
        }

        tsk_vs_close(vs);
    } 
    else 
    {
        TSK_FS_INFO* pfs = tsk_fs_open_img(img, 0, TSK_FS_TYPE_DETECT);

        qInfo() << "[TskWalker] Probing Image-Level Filesystem, Result=" << (pfs ? "OK" : "No");

        if (pfs) 
        {
            fsVector.emplace_back(pfs, 0, fsIndex[0]++);
        }
    }

    if (fsVector.empty()) 
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
    for (auto& [fsInfo, partitionID, fsIndex] : fsVector) 
    {
        walk_ctx.partitionId = partitionID;
        walk_ctx.fsIndex = fsIndex;

        // Create a Node for the FS Root
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

        uint64_t fsIdx64 = (uint64_t)walk_ctx.fsIndex;
        QString partitionPath = QString::number((qulonglong)partitionID) + ":" + QString::number((qulonglong)fsIdx64) + ":/";
        QByteArray partitionUtf8 = partitionPath.toUtf8();

        MDB_val pk{ static_cast<size_t>(partitionUtf8.size()), const_cast<char*>(partitionUtf8.data()) };
        MDB_val pv{ sizeof(part_root_id), &part_root_id };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_paths_dbi, &pk, &pv, 0);

        uint64_t keyCombined = ((uint64_t)partitionID << 32) | (uint64_t)fsIdx64;
        walk_ctx.root_node_ids[keyCombined] = part_root_id;


        // UNALLOCATED Path
        uint64_t unalloc_node = walk_ctx.next_node_id++;
        NodeMetadata unallocMeta{};
        unallocMeta.is_dir = 1;
        unallocMeta.inode = 0;
        unallocMeta.size = 0;
        unallocMeta.type = 0;
        unallocMeta.ctime = 0;
        unallocMeta.mtime = 0;
        unallocMeta.status = 1; // Unallocated
        MDB_val unk{ sizeof(unalloc_node), &unalloc_node };
        MDB_val unv{ sizeof(unallocMeta), &unallocMeta };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_nodes_dbi, &unk, &unv, 0);

        walk_ctx.tmp_key.clear();
        walk_ctx.tmp_key.append(QByteArray::number((qulonglong)partitionID));
        walk_ctx.tmp_key.append(':');
        walk_ctx.tmp_key.append(QByteArray::number((qulonglong)fsIdx64));
        walk_ctx.tmp_key.append(':');
        walk_ctx.tmp_key.append("UNALLOCATED");
        MDB_val upk{ static_cast<size_t>(walk_ctx.tmp_key.size()), const_cast<char*>(walk_ctx.tmp_key.data()) };
        MDB_val upv{ sizeof(unalloc_node), &unalloc_node };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_paths_dbi, &upk, &upv, 0);

        MDB_val uck{ sizeof(part_root_id), &part_root_id };
        MDB_val ucv{ sizeof(unalloc_node), &unalloc_node };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_children_dbi, &uck, &ucv, 0);
        walk_ctx.unalloc_node_ids[keyCombined] = unalloc_node;

        // DELETED Path
        uint64_t deleted_node = walk_ctx.next_node_id++;
        NodeMetadata deletedMeta{};
        deletedMeta.is_dir = 1;
        deletedMeta.inode = 0;
        deletedMeta.size = 0;
        deletedMeta.type = 0;
        deletedMeta.ctime = 0;
        deletedMeta.mtime = 0;
        deletedMeta.status = 2; // Deleted
        MDB_val dnk{ sizeof(deleted_node), &deleted_node };
        MDB_val dnv{ sizeof(deletedMeta), &deletedMeta };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_nodes_dbi, &dnk, &dnv, 0);

        walk_ctx.tmp_key.clear();
        walk_ctx.tmp_key.append(QByteArray::number((qulonglong)partitionID));
        walk_ctx.tmp_key.append(':');
        walk_ctx.tmp_key.append(QByteArray::number((qulonglong)fsIdx64));
        walk_ctx.tmp_key.append(':');
        walk_ctx.tmp_key.append("DELETED");
        MDB_val dpk{ static_cast<size_t>(walk_ctx.tmp_key.size()), const_cast<char*>(walk_ctx.tmp_key.data()) };
        MDB_val dpv{ sizeof(deleted_node), &deleted_node };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_paths_dbi, &dpk, &dpv, 0);

        MDB_val dck{ sizeof(part_root_id), &part_root_id };
        MDB_val dcv{ sizeof(deleted_node), &deleted_node };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_children_dbi, &dck, &dcv, 0);
        walk_ctx.deleted_node_ids[keyCombined] = deleted_node;

        // ORPHAN Path
        uint64_t orphan_node = walk_ctx.next_node_id++;
        NodeMetadata orphanMeta{};
        orphanMeta.is_dir = 1;
        orphanMeta.inode = 0;
        orphanMeta.size = 0;
        orphanMeta.type = 0;
        orphanMeta.ctime = 0;
        orphanMeta.mtime = 0;
        orphanMeta.status = 3; // Orphan
        MDB_val ok{ sizeof(orphan_node), &orphan_node };
        MDB_val ov{ sizeof(orphanMeta), &orphanMeta };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_nodes_dbi, &ok, &ov, 0);

        walk_ctx.tmp_key.clear();
        walk_ctx.tmp_key.append(QByteArray::number((qulonglong)partitionID));
        walk_ctx.tmp_key.append(':');
        walk_ctx.tmp_key.append(QByteArray::number((qulonglong)fsIdx64));
        walk_ctx.tmp_key.append(':');
        walk_ctx.tmp_key.append("ORPHAN");
        MDB_val opk{ static_cast<size_t>(walk_ctx.tmp_key.size()), const_cast<char*>(walk_ctx.tmp_key.data()) };
        MDB_val opv{ sizeof(orphan_node), &orphan_node };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_paths_dbi, &opk, &opv, 0);

        MDB_val ock{ sizeof(part_root_id), &part_root_id };
        MDB_val ocv{ sizeof(orphan_node), &orphan_node };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_children_dbi, &ock, &ocv, 0);
        walk_ctx.orphan_node_ids[keyCombined] = orphan_node;
        

        unsigned char iNodeArray[24];
        uint64_t part = (uint64_t)partitionID;
        uint64_t root_inode = fsInfo->root_inum;
        uint64_t fsIdxLocal = fsIdx64;
        memcpy(iNodeArray, &part, sizeof(part));
        memcpy(iNodeArray + sizeof(part), &fsIdxLocal, sizeof(fsIdxLocal));
        memcpy(iNodeArray + sizeof(part) + sizeof(fsIdxLocal), &root_inode, sizeof(root_inode));
        MDB_val ik{ sizeof(iNodeArray), iNodeArray };
        MDB_val iv{ sizeof(part_root_id), &part_root_id };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_inode_map_dbi, &ik, &iv, 0);
    

        MDB_val ck{ sizeof(top_root_id), &top_root_id };
        MDB_val cv{ sizeof(part_root_id), &part_root_id };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_children_dbi, &ck, &cv, 0);

        uint8_t walkResult = tsk_fs_dir_walk(fsInfo, fsInfo->root_inum,
            static_cast<TSK_FS_DIR_WALK_FLAG_ENUM>(TSK_FS_DIR_WALK_FLAG_RECURSE | TSK_FS_DIR_WALK_FLAG_ALLOC | TSK_FS_DIR_WALK_FLAG_UNALLOC),
            file_callback, &walk_ctx);

        if (walkResult != 0)
            qWarning() << "[TskWalker] Walk Error on Partition" << partitionID << ":" << tsk_error_get();

        tsk_fs_close(fsInfo);

        qInfo() << "[TskWalker] Directory Walk Returned for Partition" << partitionID;

    }



    for (const RawPartitionInfo& part : rawPartitions)
    {
        uint64_t raw_node_id = walk_ctx.next_node_id++;

        NodeMetadata rawNodeMetadata{};
        rawNodeMetadata.is_dir  = 1;
        rawNodeMetadata.status  = 4; // Raw — no filesystem
        rawNodeMetadata.size    = part.sizeBytes;
        rawNodeMetadata.inode   = 0;
        rawNodeMetadata.type    = 0;
        rawNodeMetadata.ctime   = 0;
        rawNodeMetadata.mtime   = 0;

        MDB_val rnk{ sizeof(raw_node_id), &raw_node_id };
        MDB_val rnv{ sizeof(rawNodeMetadata), &rawNodeMetadata };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_nodes_dbi, &rnk, &rnv, 0);

        walk_ctx.tmp_key.clear();
        walk_ctx.tmp_key.append(QByteArray::number((qulonglong)part.addr));
        walk_ctx.tmp_key.append(":RAW");
        if (!part.desc.empty())
        {
            walk_ctx.tmp_key.append('(');
            walk_ctx.tmp_key.append(part.desc.c_str(), (int)part.desc.size());
            walk_ctx.tmp_key.append(')');
        }
        MDB_val rpk{ static_cast<size_t>(walk_ctx.tmp_key.size()), const_cast<char*>(walk_ctx.tmp_key.data()) };
        MDB_val rpv{ sizeof(raw_node_id), &raw_node_id };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_paths_dbi, &rpk, &rpv, 0);

        MDB_val rck{ sizeof(top_root_id), &top_root_id };
        MDB_val rcv{ sizeof(raw_node_id), &raw_node_id };
        mdb_put(walk_ctx.lmdb_txn, walk_ctx.lmdb_children_dbi, &rck, &rcv, 0);

        qInfo() << "[TskWalker] Raw Partition Node Created: addr=" << part.addr
                << "desc=" << part.desc.c_str()
                << "size=" << part.sizeBytes << "bytes"
                << "node_id=" << raw_node_id;
    }

    if (walk_ctx.lmdb_txn) 
    {
        int rc = mdb_txn_commit(walk_ctx.lmdb_txn);
        if (rc != 0) 
        {
            qWarning() << "[TskWalker] Failed to Commit LMDB txn:" << mdb_strerror(rc);
            mdb_txn_abort(walk_ctx.lmdb_txn);
        }
    }

    if (nodes_dbi) mdb_dbi_close(env, nodes_dbi);
    if (children_dbi) mdb_dbi_close(env, children_dbi);
    if (paths_dbi) mdb_dbi_close(env, paths_dbi);
    if (inode_map_dbi) mdb_dbi_close(env, inode_map_dbi);

    tsk_img_close(img);

    qInfo() << "[TskWalker] ########################################################";
    qInfo() << "[TskWalker] Disk Walk Completed:";
    qInfo() << "[TskWalker] Directories:" << walk_ctx.dir_count;
    qInfo() << "[TskWalker] Files:" << walk_ctx.file_count;
    qInfo() << "[TskWalker] Total Nodes:" << (walk_ctx.file_count + walk_ctx.dir_count + 1);    
    qInfo() << "[TskWalker] ########################################################";


    return true;
}

