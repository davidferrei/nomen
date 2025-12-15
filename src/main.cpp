#include "ui/windows/mainwindow.h"
#include "ui/dialogs/startupdialog.h"
#include "core/logging/logger.h"

#include <tsk/libtsk.h>
#include <ibpp/core/ibpp.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
    setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
    setenv("FIREBIRD_LOCK", "/tmp/firebird_lock", 1);
    mkdir("/tmp/firebird_lock", 0700);;

    QApplication a(argc, argv);

    //Program Setup
        //TODO: Add Setup info like the DocumentViewer Example

    //Debug Setup
    qInstallMessageHandler(nomenLogger);


    //Program Start
    startupDialog startup;
    if (startup.exec() != QDialog::Accepted) return 0;

    MainWindow w;
    w.show();

    //TSK Version
    const char* tskVersion = tsk_version_get_str();
    qInfo() << "The Sleuth Kit version:" << tskVersion;

    //


    try
    {
        const std::string dbPath = "/tmp/sql_ok.fdb";
        const std::string user   = "SYSDBA";
        const std::string pass   = "masterkey";

        IBPP::Database db = IBPP::DatabaseFactory(
            "",
            dbPath,
            user,
            pass
            );

        if (!std::filesystem::exists(dbPath))
        {
            qDebug() << "Database does not exist, creating...";
            db->Create(3);          // SQL dialect 3
            db->Disconnect();       // Create() auto-connects
        }
        else
        {
            qDebug() << "Database exists, connecting...";
            db->Connect();
        }
    }
    catch (const IBPP::Exception& e)
    {
        std::cerr << e.ErrorMessage() << std::endl;
    }



    return a.exec();

}
