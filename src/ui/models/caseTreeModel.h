#ifndef CASETREEMODEL_H
#define CASETREEMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QString>
#include <QIcon>
#include <QRegularExpression>


#include <memory>

#include "core/index/repo/IndexDBManager.h"

//TODO: Display Child Counter

enum class TreeItemType
{
    Root,
    DataSources_Root,
    DataSource,
    FS_Structure,
    Evidence_Root,
    Evidence_Tags,
    Evidence_Groups,
    Views_Root,
    IngestResults
};

struct TreeItem
{
    TreeItemType type;

    QString name;
    quint64 id = 0;
    int dsId = -1;
    bool isLazyLoaded = false;

    TreeItem* parent = nullptr;
    QList<TreeItem*> children;
    

    ~TreeItem() { qDeleteAll(children); }
};

class CaseTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CaseTreeModel(QObject *parent = nullptr);
    ~CaseTreeModel() override;


    //Note: Must be Called Before the Model is Used
    void addDataSource(int dsId, const QString &label, const QString &lmdbPath);

    // QAbstractItemModel Overrides
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    
    bool hasChildren(const QModelIndex &parent = {}) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

private:
    void buildStaticRootTreeItems();
    void populateDSTreeItemChildren(TreeItem *item);
    TreeItem* itemFromIndex(const QModelIndex &idx) const;

    TreeItem* root;
    TreeItem* dataSources;
    QMap<int, std::shared_ptr<IndexDBManager>> dbManagers;
};

#endif // CASETREEMODEL_H
