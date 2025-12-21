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
    MainWindow(QWidget *parent, const CaseManager& caseManager);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    const CaseManager& caseManager;
};
#endif // MAINWINDOW_H
