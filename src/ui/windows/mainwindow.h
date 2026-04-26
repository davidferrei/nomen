#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/caseManager/casemanager.h"
#include "core/jobManager/jobManager.h"
#include "core/caseDBManager/repositories/datasourceRepository.h"
#include "../../core/index/repo/IndexDBManager.h"
#include "core/extensions/ingestModuleFactory.h"


#include "../widgets/DraggableTabBar.h"
#include "../widgets/NotificationWidget.h"
#include "../widgets/DraggableTabWidget.h"
#include "../widgets/fileViewers/PlainTextViewer.h"
#include "../widgets/metadataViewer/MetadataViewer.h"

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
#include <QFileInfo>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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

    // DATA
    CaseManager& caseManager;
    JobManager* jobManager;
    QMap<int, QString> dataSourceNames;

    int navigationSelectedDSId = -1;
    QString navigationCurrentPath;

    // Tree
    CaseTreeModel* caseTreeModel;

    // FS Table
    ItemTableModel* itemTableModel;
    QSortFilterProxyModel* itemTableProxy;

    // Viewers
    PlainTextViewer* plainTextViewer = nullptr;
    MetadataViewer* metadataViewer = nullptr;

    // LOGIC
    void loadDataSources();

    void setUIWidgetsSizes();
    void setupUiElements();
    void setupSignalConnections();
    void setupJobSignalConnections();
    void setupUiSignalConnections();


private slots:
    void onCaseTreeNodeActivated(int dsId, const QString &path);
    void onFSUpPushButtonClicked();
    void onFSTableRowClicked(const QModelIndex &proxyIndex);
    void onFSTableRowDoubleClicked(const QModelIndex &proxyIndex);


};
#endif // MAINWINDOW_H
