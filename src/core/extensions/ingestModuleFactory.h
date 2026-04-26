#ifndef INGESTMODULEFACTORY_H
#define INGESTMODULEFACTORY_H

#include "core/extensions/ingestModuleInterface.h"

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

    static IngestModuleFactory &instance();

    // Query API
    QStringList moduleNamesList() const;
    ModuleList modulesList() const;
    AbstractIngestModule *fetchModule(const QString &moduleName) const;
    QJsonObject getModuleMetadata(const QString &moduleName) const;

    // Load / Unload
    void loadModulePlugins();
    void unloadModulePlugins();

private:
    void addModule(QObject *moduleObject, const QJsonObject &metadata = QJsonObject());

    QHash<QString, AbstractIngestModule *> modulesHashTable;

    IngestModuleFactory(const IngestModuleFactory &) = delete;
    IngestModuleFactory &operator=(const IngestModuleFactory &) = delete;
    
};

#endif // INGESTMODULEFACTORY_H
