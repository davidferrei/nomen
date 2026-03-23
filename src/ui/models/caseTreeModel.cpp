#include "caseTreeModel.h"


CaseTreeModel::CaseTreeModel(QObject *parent) : QAbstractItemModel(parent) , root(new TreeItem{ TreeItemType::Root, QString(), 0, -1, false }), dataSources(nullptr)
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
    auto *ingestResults = new TreeItem{ TreeItemType::IngestResults, QStringLiteral("Ingest Results"), 0, -1, false };
    ingestResults->parent = root;
    ingestResults->isLazyLoaded = false;
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

    dbManagers.insert(dsId, dsIndexDB);

    quint64 rootNodeId = 0;
    auto rootResult = dsIndexDB->getNodeByPath(QStringLiteral("/"));
    if (rootResult.first)
        rootNodeId = rootResult.second;

    int row = dataSources->children.size();
    beginInsertRows(index(0, 0), row, row);

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

    return {};
}

//############################################################################################
// Lazy loading

bool CaseTreeModel::hasChildren(const QModelIndex &parent) const
{
    TreeItem *item = itemFromIndex(parent);
    if (!item)
        return false;

    if (item->isLazyLoaded && (item->type == TreeItemType::DataSource || item->type == TreeItemType::FS_Structure))
        return true;

    return !item->children.isEmpty();
}

bool CaseTreeModel::canFetchMore(const QModelIndex &parent) const
{
    TreeItem *item = itemFromIndex(parent);
    if (!item)
        return false;

    return item->isLazyLoaded && (item->type == TreeItemType::DataSource || item->type == TreeItemType::FS_Structure);
}

void CaseTreeModel::fetchMore(const QModelIndex &parent)
{
    TreeItem *item = itemFromIndex(parent);
    if (!item || !item->isLazyLoaded)
        return;

    populateDSTreeItemChildren(item);
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

    auto dsIndexIterator = dbManagers.find(item->dsId);
    if (dsIndexIterator == dbManagers.end())
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

    int parentRow = item->parent->children.indexOf(item);
    if (parentRow < 0)
    {
        qWarning() << "[CaseTreeModel] Cannot Populate Children: Item not Found in Parent's Children List" << item->name << "(ID:" << item->id << ")";
        return;
    }

    auto parentIdx = createIndex(parentRow, 0, item);

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

        auto childMeta = dsIndexDB->getNode(childDirId);
        auto childPath = dsIndexDB->getPathForNode(childDirId);

        if (item->type == TreeItemType::DataSource && childPath.first)
        {
            name = childPath.second;
        }
        else
        {
            if (childMeta.first)
            {

                const int status = childMeta.second.status;
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
                name = QStringLiteral("Node %1").arg(childDirId);
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
}

//############################################################################################
// Helpers

TreeItem* CaseTreeModel::itemFromIndex(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return root;

    return static_cast<TreeItem*>(idx.internalPointer());
}
