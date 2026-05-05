#include "ingestResultDBInstance.h"

IngestResultDBInstance::IngestResultDBInstance(IBPP::Database db, qint64 jobIdArg, QObject *parent) : QObject(parent), itemRepo(db), fileRepo(db), jobId(jobIdArg) {}


qint64 IngestResultDBInstance::createFolderItem(const QString &name)
{
    IngestResultItemData ingestItem;
    ingestItem.jobId = jobId;
    ingestItem.parentItemId = -1;
    ingestItem.itemType = IngestResultItemData::ItemType::Folder;
    ingestItem.itemName = name;

    if (!itemRepo.insert(ingestItem)) 
    {
        qWarning() << "[IngestResultDBInstance] Failed to Create Folder Item: " << name;
        return -1;
    }
    return ingestItem.id;
}

qint64 IngestResultDBInstance::createFolderItem(qint64 parentId, const QString &name)
{
    IngestResultItemData ingestItem;
    ingestItem.jobId = jobId;
    ingestItem.parentItemId = parentId < 0 ? -1 : parentId;
    ingestItem.itemType = IngestResultItemData::ItemType::Folder;
    ingestItem.itemName = name;

    if (!itemRepo.insert(ingestItem)) 
    {
        qWarning() << "[IngestResultDBInstance] Failed to Create Folder Item: " << name << " parent:" << parentId;
        return -1;
    }
    return ingestItem.id;
}

qint64 IngestResultDBInstance::createTextItem(qint64 parentId, const QString &label, const QString &value)
{
    IngestResultItemData ingestItem;
    ingestItem.jobId = jobId;
    ingestItem.parentItemId = parentId < 0 ? -1 : parentId;
    ingestItem.itemType = IngestResultItemData::ItemType::Text;
    if (label.isEmpty() && !value.isEmpty())
        ingestItem.itemName = value;
    else if (!label.isEmpty() && value.isEmpty())
        ingestItem.itemName = label;
    else
        ingestItem.itemName = QString("%1: %2").arg(label, value);

    if (!itemRepo.insert(ingestItem)) 
    {
        qWarning() << "[IngestResultDBInstance] Failed to Create Text Item: " << label << " = " << value;
        return -1;
    }
    return ingestItem.id;
}

qint64 IngestResultDBInstance::createFileItem(qint64 datasourceId, qint64 partitionId, qint64 filesystemId, const QString &lmdbPath, const QString &notes)
{
    IngestFileData ingestFile;
    ingestFile.datasourceId = datasourceId;
    ingestFile.partitionId = partitionId;
    ingestFile.filesystemId = filesystemId;
    ingestFile.lmdbPath = lmdbPath;
    ingestFile.lmdbPathHash = QString::fromLatin1(QCryptographicHash::hash(lmdbPath.toUtf8(), QCryptographicHash::Sha256).toHex());
    ingestFile.notes = notes;

    if (!fileRepo.insert(ingestFile)) 
    {
        qWarning() << "[IngestResultDBInstance] Failed to Create File Item: " << lmdbPath;
        return -1;
    }
    return ingestFile.id;
}
