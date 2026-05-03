#include "caseTreeModel.h"

CaseTreeModel::CaseTreeModel(QObject *parent, CaseManager *caseManager) : QAbstractItemModel(parent) , root(new TreeItem{ TreeItemType::Root, QString(), 0, -1, false }), dataSources(nullptr), caseManagerPtr(caseManager)
{
    buildStaticRootTreeItems();
}

CaseTreeModel::~CaseTreeModel()
{
    delete root;
}

//############################################################################################
// Functions

void CaseTreeModel::buildStaticRootTreeItems()
{
    // Data Sources
    dataSources = new TreeItem{ TreeItemType::DataSources_Root, QStringLiteral("Data Sources"), 0, -1, false };
    dataSources->parent = root;
    dataSources->isLazyLoaded = false;
    root->children.append(dataSources);

    // Evidence
    auto *evidence = new TreeItem{ TreeItemType::Evidence_Root, QStringLiteral("Evidence"), 0, -1, false };
    evidence->parent = root;
    evidence->isLazyLoaded = false;
    root->children.append(evidence);

    auto *tags = new TreeItem{ TreeItemType::Evidence_Tags, QStringLiteral("Tags"), 0, -1, false };
    tags->parent = evidence;
    tags->isLazyLoaded = false;
    evidence->children.append(tags);

    auto *groups = new TreeItem{ TreeItemType::Evidence_Groups, QStringLiteral("Groups"), 0, -1, false };
    groups->parent = evidence;
    groups->isLazyLoaded = false;
    evidence->children.append(groups);

    // Views
    auto *views = new TreeItem{ TreeItemType::Views_Root, QStringLiteral("Views"), 0, -1, false };
    views->parent = root;
    views->isLazyLoaded = false;
    root->children.append(views);

    // Ingest Results
    auto *ingestResults = new TreeItem{ TreeItemType::IngestResults, QStringLiteral("Ingest Results"), 0, -1, true };
    ingestResults->parent = root;
    ingestResults->isLazyLoaded = true;
    root->children.append(ingestResults);
}


void CaseTreeModel::addDataSource(int dsId, const QString &label, const QString &lmdbPath)
{
    auto dsIndexDB = std::make_shared<IndexDBManager>(lmdbPath);
    if (!dsIndexDB->open())
    {
        qWarning() << "[CaseTreeModel] Failed to Open LMDB for DS" << dsId << "at" << lmdbPath;
        return;
    }

    indexDBInstancesMap.insert(dsId, dsIndexDB);

    quint64 rootNodeId = 0;
    auto rootResult = dsIndexDB->getNodeByPath(QStringLiteral("/"));
    if (rootResult.first)
        rootNodeId = rootResult.second;

    int row = dataSources->children.size();
    QModelIndex parentIdx = indexForItem(dataSources);
    beginInsertRows(parentIdx, row, row);

    auto *dsItem = new TreeItem{ TreeItemType::DataSource, label, rootNodeId, dsId, true };
    dsItem->parent = dataSources;
    dsItem->dsId = dsId;
    dsItem->id = rootNodeId;
    dsItem->isLazyLoaded = true; 
    dataSources->children.append(dsItem);

    endInsertRows();
}

//############################################################################################
// QAbstractItemModel Overrides

QModelIndex CaseTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    TreeItem *parentItem = itemFromIndex(parent);
    if (!parentItem || row >= parentItem->children.size())
        return {};

    return createIndex(row, column, parentItem->children.at(row));
}

QModelIndex CaseTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return {};

    auto *childItem  = static_cast<TreeItem*>(child.internalPointer());
    TreeItem *parentItem = childItem ? childItem->parent : nullptr;
    if (!parentItem || parentItem == root)
        return {};

    TreeItem *grandparent = parentItem->parent;
    if (!grandparent)
        return {};

    int row = grandparent->children.indexOf(parentItem);
    if (row < 0)
        return {};

    return createIndex(row, 0, parentItem);
}

int CaseTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *item = itemFromIndex(parent);
    return item ? item->children.size() : 0;
}

int CaseTreeModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}


QVariant CaseTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto *item = static_cast<TreeItem*>(index.internalPointer());
    if (!item)
        return {};

    if (role == Qt::DisplayRole)
        return item->name;

    if (role == Qt::DecorationRole)
    {
        QIcon ic;
        switch (item->type)
        {
            case TreeItemType::DataSources_Root:
                ic = QIcon::fromTheme(QStringLiteral("computer"));
                break;

            case TreeItemType::DataSource:
                ic = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
                break;

            case TreeItemType::FS_Structure:
                if (item->parent && item->parent->type == TreeItemType::DataSource)
                {
                    ic = QIcon::fromTheme(QStringLiteral("drive-partition"));
                }
                else
                {
                    ic = QIcon::fromTheme(QStringLiteral("folder"));
                }
                break;

            case TreeItemType::Evidence_Root:
                ic = QIcon::fromTheme(QStringLiteral("folder-documents"));
                break;

            case TreeItemType::Evidence_Tags:
                ic = QIcon::fromTheme(QStringLiteral("tag"));
                break;

            case TreeItemType::Evidence_Groups:
                ic = QIcon::fromTheme(QStringLiteral("object-group"));
                break;

            case TreeItemType::Views_Root:
                ic = QIcon::fromTheme(QStringLiteral("edit-find"));
                break;

            case TreeItemType::IngestResults:
                ic = QIcon::fromTheme(QStringLiteral("system-run"));
                break;
            
            case TreeItemType::IngestJob:
                ic = QIcon::fromTheme(QStringLiteral("view-list"));
                break;

            case TreeItemType::IngestResultItem:

                if (item->isFolder || !item->children.isEmpty())
                {
                    ic = QIcon::fromTheme(QStringLiteral("folder"));
                }
                else
                {
                    // ic = QIcon::fromTheme(QStringLiteral("text-x-generic"));
                    ic = QIcon::fromTheme(QStringLiteral("folder-notes"));
                }
                break;
        }

        if (!ic.isNull())
            return ic;

        return {};
    }

    return {};
}

//############################################################################################
// Lazy loading

bool CaseTreeModel::hasChildren(const QModelIndex &parent) const
{
    TreeItem *item = itemFromIndex(parent);
    if (!item)
        return false;
    if (item->isLazyLoaded)
        return true;

    return !item->children.isEmpty();
}

bool CaseTreeModel::canFetchMore(const QModelIndex &parent) const
{
    TreeItem *item = itemFromIndex(parent);
    if (!item)
        return false;
    return item->isLazyLoaded;
}

void CaseTreeModel::fetchMore(const QModelIndex &parent)
{
    TreeItem *item = itemFromIndex(parent);
    if (!item || !item->isLazyLoaded)
        return;
    
    // Ingest Results
    if (item->type == TreeItemType::IngestResults) 
    { 
        fetchIngestJobs(item); 
        return; 
    }
    if (item->type == TreeItemType::IngestJob || item->type == TreeItemType::IngestResultItem) 
    { 
        fetchIngestResultItems(item); 
        return; 
    }

    // DataSources
    if (item->type == TreeItemType::DataSource || item->type == TreeItemType::FS_Structure)
    {
        populateDSTreeItemChildren(item);
    }
}

//FIXME: Handle Special Nodes

void CaseTreeModel::populateDSTreeItemChildren(TreeItem *item)
{
    if (!item) return;

    item->isLazyLoaded = false;

    if (item->dsId < 0)
    {
        TreeItem *ancestorItem = item->parent;
        while (ancestorItem && ancestorItem->dsId < 0)
            ancestorItem = ancestorItem->parent;

        if (ancestorItem)
            item->dsId = ancestorItem->dsId;
        else
        {
            qWarning() << "[CaseTreeModel] Unable to Find Parent DataSource ID for Item" << item->name << "(ID:" << item->id << ")";
            return;
        }
    }

    auto dsIndexIterator = indexDBInstancesMap.find(item->dsId);
    if (dsIndexIterator == indexDBInstancesMap.end())
    {
        qWarning() << "[CaseTreeModel] No IndexDBManager Found for DS ID" << item->dsId << "While Populating Children for Item" << item->name << "(ID:" << item->id << ")";
        return;
    }

    IndexDBManager *dsIndexDB = dsIndexIterator.value().get();
    if (!dsIndexDB)
    {
        qWarning() << "[CaseTreeModel] IndexDBManager pointer is null for DS" << item->dsId;
        return;
    }

    if (!item->parent)
    {
        qWarning() << "[CaseTreeModel] Cannot Populate Children: Item has no Parent" << item->name << "(ID:" << item->id << ")";
        return;
    }

    int parentRow = item->parent ? item->parent->children.indexOf(item) : -1;
    if (item->parent && parentRow < 0)
    {
        qWarning() << "[CaseTreeModel] Cannot Populate Children: Item not Found in Parent's Children List" << item->name << "(ID:" << item->id << ")";
        item->isLazyLoaded = false;
        return;
    }

    QModelIndex parentIdx = indexForItem(item);

    auto ItemChildDirs = dsIndexDB->getChildDirectories(item->id);
    if (!ItemChildDirs.first)
    {
        qWarning() << "[CaseTreeModel] Failed to Get Child Directories for Item" << item->name << "(ID:" << item->id << ") in DS ID" << item->dsId;
        return;
    }


    if (ItemChildDirs.second.isEmpty()) return;

    struct ResolvedChild { QString name; quint64 id; };
    QVector<ResolvedChild> resolvedChildren;
    resolvedChildren.reserve(ItemChildDirs.second.size());

    for (quint64 childDirId : ItemChildDirs.second)
    {
        QString name;

        auto childMetadata = dsIndexDB->getNode(childDirId);
        auto childPath = dsIndexDB->getPathForNode(childDirId);

        if (item->type == TreeItemType::DataSource && childPath.first)
        {
            name = childPath.second;
        }
        else
        {
            if (childMetadata.first)
            {

                const int status = childMetadata.second.status;
                if (status == 1 || status == 2 || status == 3)
                {
                    if (childPath.first && !childPath.second.isEmpty())
                    {
                        name = childPath.second;
                    }
                    else
                    {
                        switch (status)
                        {
                            case 1: name = QStringLiteral("UNALLOCATED"); break;
                            case 2: name = QStringLiteral("DELETED");     break;
                            case 3: name = QStringLiteral("ORPHAN");      break;
                            default: break;
                        }
                    }
                }
            }

            if (name.isEmpty() && childPath.first)
            {
                const QString &full = childPath.second;
                if (full == QStringLiteral("/"))
                    name = QStringLiteral("/");
                else
                {
                    int lastSlash = full.lastIndexOf('/');
                    if (lastSlash >= 0 && lastSlash < full.size() - 1)
                        name = full.mid(lastSlash + 1);
                    else
                        name = full;
                }
            }

            if (name.isEmpty())
            {
                name = QStringLiteral("Node - %1").arg(childDirId);
                qWarning() << "[CaseTreeModel] Could not Resolve Name for Child Directory" << childDirId << "in DS ID" << item->dsId;
            }
        }

        resolvedChildren.append({ name, childDirId });
    }

    if (resolvedChildren.isEmpty())
        return;

    int firstRow = item->children.size();
    int lastRow = firstRow + resolvedChildren.size() - 1;
    beginInsertRows(parentIdx, firstRow, lastRow);

    for (const ResolvedChild &rc : resolvedChildren)
    {
        auto *child = new TreeItem{ TreeItemType::FS_Structure, rc.name, rc.id, item->dsId, true };
        child->parent       = item;
        child->dsId         = item->dsId;
        child->id           = rc.id;
        child->isLazyLoaded = true;
        item->children.append(child);
    }

    endInsertRows();
    return;
}

//############################################################################################
// Ingest Results 

//TODO: Move all these calls to a Thread
//TODO: Add a Small Loader Item/Icon while we fetch data

void CaseTreeModel::fetchIngestJobs(TreeItem *item)
{
    PerformanceTimer timer("CaseTreeModel: fetchIngestJobs");

    if (!item) 
    { 
        qWarning() << "[CaseTreeModel] fetchIngestJobs Called with NULL Item"; 
        return;
    }
    if (!caseManagerPtr) 
    { 
        qWarning() << "[CaseTreeModel] No CaseManager Available;"; 
        item->isLazyLoaded = false; 
        return; 
    }

    item->isLazyLoaded = false;

    QList<IngestJobHistoryData> jobs = caseManagerPtr->getIngestJobs();
    if (jobs.isEmpty()) 
    { 
        qWarning() << "[CaseTreeModel] No Ingest Jobs Found for Case";
        return;
    }

    QModelIndex parentIdx = indexForItem(item);
    int firstRow = item->children.size();
    int lastRow = firstRow + jobs.size() - 1;
    beginInsertRows(parentIdx, firstRow, lastRow);
    for (const IngestJobHistoryData &j : jobs)
    {
        QString label = QStringLiteral("[%1] %2 v%3 - %4").arg(j.id).arg(j.moduleName).arg(j.moduleVersion).arg(j.startTime.toString(Qt::ISODate));
        auto *jobItem = new TreeItem{ TreeItemType::IngestJob, label, static_cast<quint64>(j.id), -1, true };
        jobItem->parent = item;
        jobItem->isLazyLoaded = true;
        item->children.append(jobItem);
    }
    endInsertRows();
    item->isLazyLoaded = false;
}

void CaseTreeModel::fetchIngestResultItems(TreeItem *item)
{
    PerformanceTimer timer("CaseTreeModel: fetchIngestResultItems");
    if (!item) 
    { 
        qWarning() << "[CaseTreeModel] fetchIngestResultItems Called with NULL Item"; 
        return; 
    }
    if (!caseManagerPtr) 
    { 
        qWarning() << "[CaseTreeModel] No CaseManager Available"; 
        item->isLazyLoaded = false; 
        return; 
    }

    item->isLazyLoaded = false;

    QList<IngestResultItemData> children;
    if (item->type == TreeItemType::IngestJob)
        children = caseManagerPtr->getIngestResultItemsForJob(static_cast<qint64>(item->id));
    else
        children = caseManagerPtr->getIngestResultChildren(static_cast<qint64>(item->id));

    if (children.isEmpty()) 
    { 
        qWarning() << "[CaseTreeModel] No Ingest Result Items Found for Item" << item->name << "(ID:" << item->id << ")";
        return; 
    }

    item->children.reserve(item->children.size() + children.size());

    QModelIndex parentIdx = indexForItem(item);
    int firstRow = item->children.size();
    int lastRow = firstRow + children.size() - 1;
    beginInsertRows(parentIdx, firstRow, lastRow);

    for (const IngestResultItemData &it : children)
    {
        auto *child = new TreeItem{ TreeItemType::IngestResultItem, it.itemName, static_cast<quint64>(it.id), static_cast<int>(it.datasourceId), true };
        child->isFolder = (it.itemType == IngestResultItemData::ItemType::Folder);
        child->parent = item;
        item->children.append(child);
    }

    endInsertRows();
    item->isLazyLoaded = false;
}

void CaseTreeModel::refreshIngestResults()
{
    if (!root) 
    {
        qWarning() << "[CaseTreeModel] refreshIngestResults: Root Item is NULL";
        return;
    }
    TreeItem *ingestRoot = nullptr;
    for (TreeItem *c : root->children) 
    {
        if (c && c->type == TreeItemType::IngestResults)
        { 
            ingestRoot = c;
            break;
        }
    }
    if (!ingestRoot)
    {
        qWarning() << "[CaseTreeModel] refreshIngestResults: Ingest Results Root Item is NULL";
        return;
    }

    if (!ingestRoot->children.isEmpty()) 
    {
        QModelIndex parentIdx = indexForItem(ingestRoot);
        int count = ingestRoot->children.size();
        beginRemoveRows(parentIdx, 0, count - 1);
        qDeleteAll(ingestRoot->children);
        ingestRoot->children.clear();
        endRemoveRows();
    }

    ingestRoot->isLazyLoaded = true;
    fetchIngestJobs(ingestRoot);
}

QModelIndex CaseTreeModel::ingestResultsIndex() const
{
    if (!root)
    {
        qWarning() << "[CaseTreeModel] ingestResultsIndex: Root Item is NULL";
        return QModelIndex();
    }
    int row = 0;

    for (TreeItem *c : root->children) 
    {
        if (c && c->type == TreeItemType::IngestResults) 
        {
            return createIndex(row, 0, c);
        }
        ++row;
    }
    
    return QModelIndex();
}

//############################################################################################
// Helpers

TreeItem* CaseTreeModel::itemFromIndex(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return root;

    return static_cast<TreeItem*>(idx.internalPointer());
}

QModelIndex CaseTreeModel::indexForItem(TreeItem *item) const
{
    if (!item) 
    {
        qWarning() << "[CaseTreeModel] indexForItem: Item is null";
        return QModelIndex();
    }
    if (!item->parent) 
    {
        qWarning() << "[CaseTreeModel] indexForItem: Item has no parent";
        return QModelIndex();
    }

    int row = item->parent->children.indexOf(item);
    if (row < 0)
    {
        qWarning() << "[CaseTreeModel] indexForItem: Item not found in parent's children:" << item->name << "(ID:" << item->id << ")";
        return QModelIndex();
    }

    return createIndex(row, 0, item);
}

QString CaseTreeModel::pathForItem(TreeItem *item) const
{
    if (!item)
    {
        qWarning() << "[CaseTreeModel] Cannot Resolve Path: Item is null";
        return {};
    }

    if (item->dsId >= 0)
    {
        auto indexDB = indexDBInstancesMap.find(item->dsId);
        if (indexDB != indexDBInstancesMap.end())
        {
            IndexDBManager *indexInstance = indexDB.value().get();
            if (indexInstance)
            {
                auto pathResult = indexInstance->getPathForNode(item->id);
                if (pathResult.first)
                    return pathResult.second;
            }
        }
    }

    //TODO: Handle Static Items
    qWarning() << "[CaseTreeModel]: Unable to Resolve Path for Item" << item->name << "(ID:" << item->id << ") with DS ID" << item->dsId;
    return QString();
}

void CaseTreeModel::onItemActivated(const QModelIndex &index)
{
    TreeItem *activatedItem = itemFromIndex(index);
    if (!activatedItem)
    {
        qWarning() << "[CaseTreeModel] Signal - onItemActivated: Invalid Index";
        return;
    }

    QString resolvedPath = pathForItem(activatedItem);

    // qDebug() << "[CaseTreeModel] Signal - nodeActivated: DS:" << activatedItem->dsId << "Path:" << resolvedPath;
    
    emit nodeActivated(activatedItem->dsId, resolvedPath);
}

QPair<bool, quint64> CaseTreeModel::getNodeIdForPath(int dsId, const QString &path) const
{
    auto indexDB = indexDBInstancesMap.find(dsId);
    if (indexDB == indexDBInstancesMap.end() || !indexDB.value())
        return {false, 0};
    return indexDB.value()->getNodeByPath(path);
}

QModelIndex CaseTreeModel::findIndex(int dsId, quint64 nodeId, const QModelIndex &parent) const
{
    TreeItem *parentItem = itemFromIndex(parent);
    if (!parentItem)
    {
        qWarning() << "[CaseTreeModel]: Invalid Parent Index";
        return {};
    }

    for (int i = 0; i < parentItem->children.size(); ++i)
    {
        TreeItem *child = parentItem->children.at(i);
        if (child->dsId == dsId && child->id == nodeId)
            return createIndex(i, 0, child);

        auto found = findIndex(dsId, nodeId, createIndex(i, 0, child));
        if (found.isValid()) return found;
    }
    return {};
}

QPair<bool, QVector<DSFSNodeItem>> CaseTreeModel::getChildrenItemsForDSPath(int dsId, const QString &path) const
{
    QVector<DSFSNodeItem> outputVector;

    if (dsId < 0 || path.isEmpty())
    {
        qWarning() << "[CaseTreeModel] getChildrenItemsForDSPath: Invalid DS ID or Path. DS ID:" << dsId << "Path:" << path;
        return QPair<bool, QVector<DSFSNodeItem>>(false, outputVector);
    }

    auto indexDB = indexDBInstancesMap.find(dsId);
    if (indexDB == indexDBInstancesMap.end())
    {
        qWarning() << "[CaseTreeModel] getChildrenItemsForDSPath: No Index DB Manager for DS with ID:" << dsId;
        return QPair<bool, QVector<DSFSNodeItem>>(false, outputVector);
    }

    IndexDBManager *indexInstance = indexDB.value().get();
    if (!indexInstance)
    {
        qWarning() << "[CaseTreeModel] getChildrenItemsForDSPath: Index DB Manager is NULL for DS with ID:" << dsId;
        return QPair<bool, QVector<DSFSNodeItem>>(false, outputVector);
    }

    auto nodeResultPair = indexInstance->getNodeByPath(path);
    if (!nodeResultPair.first)
    {
        qWarning() << "[CaseTreeModel] getChildrenItemsForDSPath: Path not Found in LMDB:" << path;
        return QPair<bool, QVector<DSFSNodeItem>>(false, outputVector);
    }

    quint64 parentNodeId = nodeResultPair.second;
    auto nodeChildrenResultPair = indexInstance->getChildren(parentNodeId);
    if (!nodeChildrenResultPair.first)
    {
        qWarning() << "[CaseTreeModel] getChildrenItemsForDSPath: Failed to get Children for Node:" << parentNodeId;
        return QPair<bool, QVector<DSFSNodeItem>>(false, outputVector);
    }

    for (quint64 node : nodeChildrenResultPair.second)
    {
        auto metaResult = indexInstance->getNode(node);
        if (!metaResult.first) 
        {
            qWarning() << "[CaseTreeModel] getChildrenItemsForDSPath: Failed to get Metadata for Child Node:" << node;
            continue;
        }

        NodeMetadata metadata = metaResult.second;
        QString childPath;
        auto nodePathResultPair = indexInstance->getPathForNode(node);
        if (nodePathResultPair.first)
        {
            childPath = nodePathResultPair.second;
        }
        else
        {
            qWarning() << "[CaseTreeModel] getChildrenItemsForDSPath: Failed to get Path for Child Node:" << node;
        }

        QString name;
        if (!childPath.isEmpty())
        {
            if (childPath == QStringLiteral("/"))
                name = QStringLiteral("/");
            else
            {
                int lastSlash = childPath.lastIndexOf('/');
                if (lastSlash >= 0 && lastSlash < childPath.size() - 1)
                    name = childPath.mid(lastSlash + 1);
                else
                    name = childPath;
            }
        }

        if (name.isEmpty())
        {
            name = QStringLiteral("Node - %1").arg(node);
            qWarning() << "[CaseTreeModel] Could not Resolve Name for Child Node" << node << "in DS ID" << dsId;
        }

        DSFSNodeItem nodeItem{ node, name, metadata, childPath };
        outputVector.append(nodeItem);
    }

    return QPair<bool, QVector<DSFSNodeItem>>(true, outputVector);
}
