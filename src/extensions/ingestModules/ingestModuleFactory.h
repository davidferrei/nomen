#ifndef INGESTMODULEFACTORY_H
#define INGESTMODULEFACTORY_H

#include "extensions/ingestModules/ingestModuleInterface.h"

#include <QHash>
#include <QList>
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QPluginLoader>
#include <QObject>

class IngestModuleFactory
{
public:
    using ModuleList = QList<AbstractIngestModule *>;

    explicit IngestModuleFactory();
    ~IngestModuleFactory();

    // Query API
    QStringList moduleNamesList() const;
    ModuleList modulesList() const;
    AbstractIngestModule *fetchModule(const QString &moduleName) const;

    // Load / Unload
    void loadModulePlugins();
    void unloadModulePlugins();

private:
    void addModule(QObject *moduleObject, const QJsonObject &metadata = QJsonObject());

    QHash<QString, AbstractIngestModule *> modulesHashTable;
    
};

#endif // INGESTMODULEFACTORY_H
