#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/caseManager/casemanager.h"
#include "core/index/indexManager.h"
#include "core/caseDBManager/repositories/datasourceRepository.h"
#include "../../core/lmdb/FSIndexRepository.h"

#include <QLabel>
#include <QMessageBox>
#include <QMainWindow>

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

    // LOGIC
    void loadDataSources();

};
#endif // MAINWINDOW_H
