#ifndef ITEMTABLEMODEL_H
#define ITEMTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QDateTime>
#include <QIcon>

#include "ui/models/caseTreeModel.h"
#include "data/case/dsFSNodeItem.h"

class ItemTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ItemTableModel(CaseTreeModel *sourceModel, QObject *parent = nullptr);

    // QAbstractItemModel overrides
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setRoot(int dsId, const QString &path);
    DSFSNodeItem itemAt(int row) const;

private:
    QVector<DSFSNodeItem> childNodeEntries;
    CaseTreeModel *sourceTreeModel;
};

#endif // ITEMTABLEMODEL_H
