#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/caseManager/casemanager.h"


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
    Ui::MainWindow *ui;
    CaseManager& caseManager;

    void setUIWidgetsSizes();
    void setupUiElements();
};
#endif // MAINWINDOW_H
