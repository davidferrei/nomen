#include "itemTableModel.h"

ItemTableModel::ItemTableModel(CaseTreeModel *sourceModel, QObject *parent): QAbstractTableModel(parent), sourceTreeModel(sourceModel) {}

int ItemTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return childNodeEntries.size();
}

int ItemTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 7;
}

QVariant ItemTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= childNodeEntries.size())
    {
        qWarning() << "[ItemTableModel] Invalid Index:" << index;
        return {};
    }

    const DSFSNodeItem &node = childNodeEntries.at(index.row());

    if (role == Qt::UserRole)
    {

        switch (index.column())
        {
            case 1:
                return node.name;
            case 3:
                return node.metadata.is_dir ? QStringLiteral("Directory") : QStringLiteral("File");
            case 2:
                if (node.metadata.is_dir)
                    return QVariant::fromValue<qint64>(-1);
                return QVariant::fromValue<qint64>(static_cast<qint64>(node.metadata.size));

            case 4:
                return QVariant::fromValue<qint64>(static_cast<qint64>(node.metadata.mtime));

            case 5:
                return QVariant::fromValue<qint64>(static_cast<qint64>(node.metadata.ctime));

            case 6:
                return QVariant::fromValue<int>(static_cast<int>(node.metadata.status));

            default:
                return {};
        }
    }

    if (index.column() == 0)
    {
        QIcon icon = node.metadata.is_dir
            ? QIcon::fromTheme(QStringLiteral("folder"))
            : QIcon::fromTheme(QStringLiteral("text-x-generic-template"));

        if (role == Qt::DecorationRole)
            return icon.isNull() ? QVariant{} : icon;

        if (role == Qt::DisplayRole && icon.isNull())
            return node.metadata.is_dir ? QStringLiteral("D") : QStringLiteral("F");

        return {};
    }

    if (role == Qt::TextAlignmentRole && index.column() == 6)
        return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case 1:
                return node.name;
            case 2:
                return node.metadata.is_dir ? QStringLiteral("-") : QString::number(node.metadata.size);
            case 3:
                return node.metadata.is_dir ? QStringLiteral("Directory") : QStringLiteral("File");
            case 4:
            {
                QDateTime mdate = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(node.metadata.mtime));
                return mdate.isValid() ? mdate.toString(Qt::ISODate) : QString();
            }
            case 5:
            {
                QDateTime cdate = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(node.metadata.ctime));
                return cdate.isValid() ? cdate.toString(Qt::ISODate) : QString();
            }
            case 6:
            {
                switch (node.metadata.status)
                {
                    case 0:
                        return QStringLiteral("Allocated");
                    case 1:
                        return QStringLiteral("Unallocated");
                    case 2:
                        return QStringLiteral("Deleted");
                    case 3:
                        return QStringLiteral("Orphan");
                    case 4:
                        return QStringLiteral("Raw");
                    default:
                        return QStringLiteral("Unknown");
                }
            }
            default:
                return {};
        }
    }

    return {};
}

QVariant ItemTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Vertical)
        return section + 1;

    switch (section)
    {
        case 0:
            return {};
        case 1:
            return QStringLiteral("Name");
        case 2:
            return QStringLiteral("Size (b)");
        case 3:
            return QStringLiteral("Node Type");
        case 4:
            return QStringLiteral("Modified Date");
        case 5:
            return QStringLiteral("Changed Date");
        case 6:
            return QStringLiteral("Status");
        default:
            return {};
    }
}

DSFSNodeItem ItemTableModel::itemAt(int row) const
{
    if (row < 0 || row >= childNodeEntries.size())
    {
        qWarning() << "[ItemTableModel] itemAt Called with Invalid Row:" << row;
        return {};
    }
    return childNodeEntries.at(row);
}

void ItemTableModel::setRoot(int dsId, const QString &path)
{
    if (!sourceTreeModel)
    {
        qWarning() << "[ItemTableModel] Source Model is not Set";
        return;
    }

    if (path.isEmpty())
    {
        qDebug() << "[ItemTableModel] setRoot Called with Empty Path;";
        return;
    }

    auto childrenNodeResultPair = sourceTreeModel->getChildrenItemsForDSPath(dsId, path);
    if (!childrenNodeResultPair.first)
    {
        qDebug() << "[ItemTableModel] No Children for Path:" << path;
        return;
    }

    beginResetModel();
    childNodeEntries.clear();
    childNodeEntries.reserve(childrenNodeResultPair.second.size());

    for (const auto &childNodes : childrenNodeResultPair.second)
    {
        DSFSNodeItem nodeItem;
        nodeItem.id = childNodes.id;
        nodeItem.name = childNodes.name.isEmpty() ? childNodes.path : childNodes.name;
        nodeItem.metadata = childNodes.metadata;
        nodeItem.path = childNodes.path;
        childNodeEntries.append(nodeItem);
    }

    endResetModel();
}
