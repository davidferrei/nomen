#include "ui/windows/mainwindow.h"
#include "ui/dialogs/startupdialog.h"
#include "core/logging/debuglogger.h"

#include <tsk/libtsk.h>

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

    //TSK Testing
    const char* tskVersion = tsk_version_get_str();
    qInfo() << "The Sleuth Kit version:" << tskVersion;


    return a.exec();

}
