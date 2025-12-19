#include "ui/windows/mainwindow.h"
#include "ui/dialogs/startupdialog.h"
#include "core/logging/logger.h"

#include <tsk/libtsk.h>

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Program Setup
        //TODO: Add Setup info like the DocumentViewer Example

    //Logger Setup
    initLogger();

    //Program Start
    startupDialog startup;
    if (startup.exec() != QDialog::Accepted) return 0;

    MainWindow w;
    w.show();

    //TSK Version
    const char* tskVersion = tsk_version_get_str();
    qInfo() << "The Sleuth Kit version:" << tskVersion;

    int app_return = a.exec();

    //Shutdown Setup
    shutdownLogger();

    // return a.exec();
    return app_return;

}
