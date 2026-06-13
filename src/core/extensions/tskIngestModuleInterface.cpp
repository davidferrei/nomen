#include "core/extensions/tskIngestModuleInterface.h"


static QString normalizeParentPath(const char* path)
{
    if (!path || path[0] == '\0')
        return QStringLiteral("/");

    size_t len = std::strlen(path);
    while (len > 1 && path[len - 1] == '/') --len;

    if (path[0] == '/')
        return QString::fromUtf8(path, static_cast<int>(len));

    QString parentFilePath;
    parentFilePath.reserve(static_cast<int>(len + 1));
    parentFilePath.append('/');
    parentFilePath.append(QString::fromUtf8(path, static_cast<int>(len)));
    return parentFilePath;
}

TSK_WALK_RET_ENUM tskIngestModuleInterface::file_callback(TSK_FS_FILE* fs_file, const char* path, void* ptr)
{
    if (!ptr)
    {
        qWarning() << "[tskIngestModuleInterface] file_callback Called with NULL Pointer";
        return TSK_WALK_CONT;
    }

    tskIngestModuleInterface* module = static_cast<tskIngestModuleInterface*>(ptr);
    if (!fs_file || !fs_file->name || !fs_file->name->name)
    {
        qWarning() << "[TskWalker] Skipping Entry: Name Information Missing"
            << "fs_file:" << (fs_file ? "Present" : "NULL")
            << "name:" << (fs_file && fs_file->name ? "Present" : "NULL")
            << "name->name:" << (fs_file && fs_file->name && fs_file->name->name ? "Present" : "NULL")
            << "path:" << (path ? path : "(NULL)");
        return TSK_WALK_CONT;
    } 

    const char* rawName = fs_file->name->name;
    if (rawName[0] == '.' && (rawName[1] == '\0' || (rawName[1] == '.' && rawName[2] == '\0')))
        return TSK_WALK_CONT;

    FileContext fileContext;
    fileContext.fs_file = fs_file;
    fileContext.parent_path = path;
    fileContext.tsk_metadata = fs_file->meta;
    fileContext.hasMetadata = (fileContext.tsk_metadata != nullptr);
    fileContext.filename = QString::fromUtf8(rawName);

    QString parent = normalizeParentPath(path);
    if (parent == "/") fileContext.filePath = QStringLiteral("/") + fileContext.filename;
    else fileContext.filePath = parent + "/" + fileContext.filename;

    try 
    {
        module->processFile(fileContext);
    } 
    catch (...) 
    {
        qWarning() << "[tskIngestModuleInterface] processFile Function Threw an Exception for" << fileContext.filePath;
    }

    return TSK_WALK_CONT;
}

bool tskIngestModuleInterface::runWalk(const QString &imagePath)
{
    qInfo() << "[tskIngestModuleInterface] Opening Image:" << imagePath;

    TSK_IMG_INFO *img = tsk_img_open_sing(imagePath.toUtf8().constData(), TSK_IMG_TYPE_DETECT, 0);
    if (!img) 
    {
        qWarning() << "[tskIngestModuleInterface] Failed to Open Image:" << imagePath << tsk_error_get();
        return false;
    }

    TSK_VS_INFO *vs = tsk_vs_open(img, 0, TSK_VS_TYPE_DETECT);

    std::vector<std::tuple<TSK_FS_INFO*, int, int>> fsVector; // fsInfo, partitionId, fsIndex
    std::unordered_map<int,int> fsIndexMap;
    struct RawPartitionInfo { TSK_PNUM_T addr; uint64_t sizeBytes; std::string desc; };
    std::vector<RawPartitionInfo> rawPartitions;

    if (vs) 
    {
        for (TSK_PNUM_T i = 0; i < vs->part_count; ++i) 
        {
            const TSK_VS_PART_INFO *part = tsk_vs_part_get(vs, i);
            if (!part) continue;

            TSK_FS_INFO *fs = tsk_fs_open_vol(part, TSK_FS_TYPE_DETECT);
            qInfo() << "[tskIngestModuleInterface] Probing Partition" << i << "FS=" << (fs ? "OK" : "No");

            if (fs) 
            {
                int partitionID = static_cast<int>(i);
                fsVector.emplace_back(fs, partitionID, fsIndexMap[partitionID]++);
            } 
            else 
            {
                rawPartitions.push_back({ part->addr, (uint64_t)part->len * vs->img_info->sector_size, (part->desc ? part->desc : "") });
            }
        }
        tsk_vs_close(vs);
    } 
    else 
    {
        TSK_FS_INFO *fs = tsk_fs_open_img(img, 0, TSK_FS_TYPE_DETECT);
        qInfo() << "[tskIngestModuleInterface] Probing Image-Level Filesystem, Result=" << (fs ? "OK" : "No");
        if (fs) fsVector.emplace_back(fs, 0, fsIndexMap[0]++);
    }

    if (fsVector.empty()) 
    {
        qWarning() << "[tskIngestModuleInterface] Failed to find any filesystems in:" << imagePath;
        tsk_img_close(img);
        return false;
    }

    for (auto& [fsInfo, partitionID, fsIndex]: fsVector) 
    {

        this->currentPartitionId = partitionID;
        this->currentFsIndex = fsIndex;

        qInfo() << "[tskIngestModuleInterface] Starting Walk on Partition" << partitionID << "fsIndex" << fsIndex;

        uint8_t walkResult = tsk_fs_dir_walk(fsInfo, fsInfo->root_inum,
            static_cast<TSK_FS_DIR_WALK_FLAG_ENUM>(TSK_FS_DIR_WALK_FLAG_RECURSE | TSK_FS_DIR_WALK_FLAG_ALLOC | TSK_FS_DIR_WALK_FLAG_UNALLOC),
            file_callback, this);

        if (walkResult != 0)
            qWarning() << "[tskIngestModuleInterface] Walk Error on Partition" << partitionID << ":" << tsk_error_get();

        tsk_fs_close(fsInfo);

        qInfo() << "[tskIngestModuleInterface] Directory Walk Returned for Partition" << partitionID;
    }

    try { this->finishWalk(); } catch (...) { qWarning() << "[tskIngestModuleInterface] finishWalk Threw an Exception"; }

    tsk_img_close(img);
    return true;
}
