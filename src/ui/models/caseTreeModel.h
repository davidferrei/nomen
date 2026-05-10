#ifndef CASETREEMODEL_H
#define CASETREEMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QString>
#include <QIcon>
#include <QRegularExpression>


#include <memory>

#include "core/index/repo/IndexDBManager.h"
#include "core/caseDBManager/caseDBThreadInstance.h"
#include "core/caseDBManager/repositories/ingestJobHistoryRepository.h"
#include "core/caseDBManager/repositories/ingestResultItemRepository.h"
#include "core/caseManager/casemanager.h"

#include "data/case/dsFSNodeItem.h"

#include "core/logging/performancemeasure.h"


class CaseManager;

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
    IngestResults,
    IngestJob,
    IngestResultItem
};

struct TreeItem
{
    TreeItemType type;

    QString name;
    quint64 id = 0;
    int dsId = -1;
    bool isLazyLoaded = false;
    IngestResultItemData::ItemType ingestItemType;

    TreeItem* parent = nullptr;
    QList<TreeItem*> children;
    

    ~TreeItem() { qDeleteAll(children); }
};

class CaseTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CaseTreeModel(QObject *parent = nullptr, CaseManager *caseManager = nullptr);
    ~CaseTreeModel() override;

    void setCasePath(const QString &path) { casePath = path; }

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

    QPair<bool, QVector<DSFSNodeItem>> getChildrenItemsForDSPath(int dsId, const QString &path) const;
    QPair<bool, quint64> getNodeIdForPath(int dsId, const QString &path) const;
    QModelIndex findIndex(int dsId, quint64 nodeId, const QModelIndex &parent = {}) const;
    
    void refreshIngestResults();
    QModelIndex ingestResultsIndex() const;


public slots:
    void onItemActivated(const QModelIndex &index);

signals:
    void nodeActivated(int dsId, const QString &path);
    void ingestItemActivated(qint64 ingestItemId);

private:
    void buildStaticRootTreeItems();
    void populateDSTreeItemChildren(TreeItem *item);
    void fetchIngestJobs(TreeItem *item);
    void fetchIngestResultItems(TreeItem *item);
    TreeItem* itemFromIndex(const QModelIndex &idx) const;
    QString pathForItem(TreeItem *item) const;
    QModelIndex indexForItem(TreeItem *item) const;

    TreeItem* root;
    TreeItem* dataSources;
    QString casePath;
    CaseManager *caseManagerPtr = nullptr;
    QMap<int, std::shared_ptr<IndexDBManager>> indexDBInstancesMap;
};

#endif // CASETREEMODEL_H
