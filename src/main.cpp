#include "ui/windows/mainwindow.h"
#include "ui/dialogs/startUpDialog/startupdialog.h"
#include "core/logging/logger.h"
#include <core/caseManager/casemanager.h>

#include <tsk/libtsk.h>

#include <QApplication>
#include <QDebug>
#include <QDate>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Program Setup
    QCoreApplication::setOrganizationName("Nomen");
    QCoreApplication::setApplicationName("Nomen");
    QGuiApplication::setDesktopFileName("nomen");
        //TODO: Add Setup info like the DocumentViewer Example

    //Logger Setup
    initLogger();

    //TSK Version
    const char* tskVersion = tsk_version_get_str();
    qInfo() << "The Sleuth Kit version:" << tskVersion;

    //Case Manager Start
    CaseManager caseManager;

    //Program Start
    startupDialog startup(nullptr,caseManager);
    if (startup.exec() != QDialog::Accepted) return 0;

    MainWindow w(nullptr,caseManager);
    w.show();

    int app_return = a.exec();

    //Shutdown Setup
    shutdownLogger();

    return app_return;

}
