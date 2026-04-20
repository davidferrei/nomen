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

#include "extensions/ingestModules/ingestModuleFactory.h"
#include <QThread>
#include <QObject>


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


    //################################################################
    //FIXME: Remove Temp Code
    IngestModuleFactory *ingestFactory = new IngestModuleFactory();
    const QStringList modules = ingestFactory->moduleNamesList();
    qInfo() << "Discovered Ingest Modules:" << modules;

    QThread *ingestWorker = nullptr;

    if (!modules.isEmpty()) 
    {
        AbstractIngestModule *mod = ingestFactory->defaultModule();
        if (!mod)
            mod = ingestFactory->fetchModule(modules.first());

        if (mod) 
        {
            qDebug() << "Starting Ingest Module Thread";

            ingestWorker = QThread::create([mod]() 
            {
                qDebug() << "Thread Started for Module:" << mod->name();

                mod->init();
                mod->process();

                qDebug() << "Thread Finished for Module:" << mod->name();
            });
            ingestWorker->start();
        }
    }

    //################################################################


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

    //FIXME: Remove Temp Code
    if (ingestWorker) 
    {
        ingestWorker->quit();
        ingestWorker->wait();
        delete ingestWorker;
    }
    delete ingestFactory;

    //Shutdown Setup
    shutdownLogger();

    return app_return;

}
