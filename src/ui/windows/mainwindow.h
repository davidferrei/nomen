#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/caseManager/casemanager.h"
#include "core/index/job/indexManager.h"
#include "core/caseDBManager/repositories/datasourceRepository.h"
#include "../../core/index/repo/IndexDBManager.h"

#include "../widgets/DraggableTabBar.h"
#include "ui/models/caseTreeModel.h"
#include "ui/models/itemTableModel.h"

#include <QLabel>
#include <QMessageBox>
#include <QMainWindow>
#include <QHeaderView>
#include <QSplitter>
#include <QTabWidget>
#include <QIcon>
#include <QSplitter>
#include <QDateTime>
#include <QMap>
#include <QSortFilterProxyModel>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QSortFilterProxyModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent, CaseManager& caseManager);
    ~MainWindow();

protected:
    void showEvent(QShowEvent *event) override;

private:
    // UI
    Ui::MainWindow *ui;


    void setUIWidgetsSizes();
    void setupUiElements();

    // DATA
    CaseManager& caseManager;
    IndexManager* indexManager;
    QMap<int, QString> dataSourceNames;

    int navigationSelectedDSId = -1;
    QString navigationCurrentPath;


    // Tree
    CaseTreeModel* caseTreeModel;

    // FS Table
    ItemTableModel* itemTableModel;
    QSortFilterProxyModel* itemTableProxy;

    // LOGIC
    void loadDataSources();

private slots:
    void onCaseTreeNodeActivated(int dsId, const QString &path);
    void onFSUpPushButtonClicked();
    void onFSTableRowClicked(const QModelIndex &proxyIndex);
    void onFSTableRowDoubleClicked(const QModelIndex &proxyIndex);


};
#endif // MAINWINDOW_H
