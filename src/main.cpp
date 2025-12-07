#include "ui/windows/mainwindow.h"
#include "ui/dialogs/startupdialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // startupDialog ui_startup;
    // ui_startup.show();

    // MainWindow ui_main_window;
    // ui_main_window.show();
    // return a.exec();

    //###

    // startupDialog startup;

    // if (startup.exec() == QDialog::Accepted)
    // {
    //     MainWindow w;
    //     w.show();
    //     return a.exec();
    // }

    // return 0;

    //###

    startupDialog startup;
    if (startup.exec() != QDialog::Accepted)
        return 0;


    MainWindow w;
    w.show();

    return a.exec();

}
