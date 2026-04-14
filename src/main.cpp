#include "ui/windows/mainwindow.h"

#include "ui/dialogs/startUpDialog/startupdialog.h"
#include "ui/wizards/caseDetailWizard/casedetailwizard.h"

#include "core/logging/logger.h"
#include <core/caseManager/casemanager.h>

#include <tsk/libtsk.h>

#include <QApplication>
#include <QDebug>
#include <QDate>
#include <QMessageBox>
#include <QDir>

#include <cstdlib>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Program Setup
    QCoreApplication::setOrganizationName("Nomen");
    QCoreApplication::setApplicationName("Nomen");
    QGuiApplication::setDesktopFileName("nomen");
        //TODO: Add Setup info like the DocumentViewer Example
    a.setWindowIcon(QIcon(":/icons/app_icon_v1.svg"));

    //Logger Setup
    initLogger();

    // Firebird Environment 
    //TODO: Windows Fix
    setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
    setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
    setenv("FIREBIRD_LOCK", "/tmp/firebird_lock", 1);

    QDir lockDir("/tmp/firebird_lock");
    if (!lockDir.exists()) lockDir.mkpath(".");

    //TSK Version
    const char* tskVersion = tsk_version_get_str();
    qInfo() << "The Sleuth Kit version:" << tskVersion;

    //Case Manager Start
    CaseManager caseManager;

    //Program Start
    while (true)
    {
        startupDialog startup(nullptr, caseManager);

        if (startup.exec() != QDialog::Accepted)
            return 0;

        if (!caseManager.getCurrentCase().isEnvInitialized)
        {
            caseDetailWizard wizard(&caseManager);

            if (wizard.exec() != QDialog::Accepted)
            {
                QMessageBox::warning(
                    nullptr,
                    "Case Initialization Required",
                    "Case Initialization Was Cancelled."
                    );

                continue;
            }
        }

        break;
    }

    MainWindow w(nullptr,caseManager);
    w.show();

    int app_return = a.exec();

    //Shutdown Setup
    shutdownLogger();

    return app_return;

}
