#include "ui/windows/mainwindow.h"
#include "ui/dialogs/startupdialog.h"
#include "core/logging/debuglogger.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Program Setup
        //TODO: Add Setup info like the Document Example

    //Debug Setup
        //TODO: This
    qInstallMessageHandler(debugLogger);


    //Program Start
    startupDialog startup;
    if (startup.exec() != QDialog::Accepted) return 0;

    MainWindow w;
    w.show();

    return a.exec();

}
