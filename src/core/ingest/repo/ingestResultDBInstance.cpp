#include "ingestResultDBInstance.h"

IngestResultDBInstance::IngestResultDBInstance(IBPP::Database db, qint64 jobIdArg, QObject *parent) : QObject(parent), repo(db), jobId(jobIdArg) {}

qint64 IngestResultDBInstance::createFolderItem(const QString &name)
{
    IngestResultItemData it;
    it.jobId = jobId;
    it.parentItemId = -1;
    it.itemType = IngestResultItemData::ItemType::Folder;
    it.itemName = name;

    if (!repo.insert(it)) 
    {
        qWarning() << "[IngestResultDBInstance] Failed to Create Folder Item: " << name;
        return -1;
    }
    return it.id;
}

qint64 IngestResultDBInstance::createTextItem(qint64 parentId, const QString &label, const QString &value)
{
    IngestResultItemData it;
    it.jobId = jobId;
    it.parentItemId = parentId < 0 ? -1 : parentId;
    it.itemType = IngestResultItemData::ItemType::Text;
    it.itemName = QString("%1: %2").arg(label, value);

    if (!repo.insert(it)) 
    {
        qWarning() << "[IngestResultDBInstance] Failed to Create Text Item: " << label << " = " << value;
        return -1;
    }
    return it.id;
}
