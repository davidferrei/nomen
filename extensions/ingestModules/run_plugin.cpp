#include <QCoreApplication>
#include <QDebug>
#include <QDir>

#include "core/logging/logger.h"
#include "core/extensions/ingestModuleFactory.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    qDebug() << "[PluginTester] Current Path:" << QDir::currentPath();
    qDebug() << "[PluginTester] Application Dir Path:" << QCoreApplication::applicationDirPath();
    qDebug() << "";

    QString tempLogPath = QDir::tempPath() + QString("/nomen_run_ingest_plugin_%1.log").arg(QCoreApplication::applicationPid());
    setLogFilePath(tempLogPath);
    initLogger();

    IngestModuleFactory factory;
    qDebug() << "[PluginTester] Available Modules:" << factory.moduleNamesList();

    qDebugNL();

    const QStringList toRun = { "TxtCounter", "DetectMIMEType" };

    for (const QString &name : toRun) 
    {
        auto *m = factory.fetchModule(name);
        if (!m) 
        {
            qWarning() << "[PluginTester] Module Not Found:" << name;
            continue;
        }

        qDebugNL();
        qDebug() << "[PluginTester] Running Module:" << m->name();

        m->init();
        m->process();

        qDebug() << "[PluginTester] Module Finished:" << m->name();
        qDebugNL();
    }

    shutdownLogger();
    return 0;
}
