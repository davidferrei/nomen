#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/caseManager/casemanager.h"
#include "core/index/job/indexManager.h"
#include "core/caseDBManager/repositories/datasourceRepository.h"
#include "../../core/index/repo/IndexDBManager.h"
#include "ui/models/caseTreeModel.h"

#include <QLabel>
#include <QMessageBox>
#include <QMainWindow>
#include <QHeaderView>
#include <QSplitter>


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

    void setUIWidgetsSizes();
    void setupUiElements();

    // DATA
    CaseManager& caseManager;
    IndexManager* indexManager;

    // Tree
    CaseTreeModel* caseTreeModel;

    // LOGIC
    void loadDataSources();

};
#endif // MAINWINDOW_H
