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

    AbstractIngestModule *module() const;

    // Query API
    QStringList moduleNamesList(bool showDefault = false) const;
    ModuleList modulesList() const;
    AbstractIngestModule *fetchModule(const QString &moduleName) const;
    AbstractIngestModule *defaultModule() const;

    // Load / Unload
    void loadModulePlugins();
    void unloadModulePlugins();

private:
    void addModule(QObject *moduleObject, const QJsonObject &metadata = QJsonObject());

    QHash<QString, AbstractIngestModule *> modulesHashTable;
    AbstractIngestModule *m_defaultModule{nullptr};
};

#endif // INGESTMODULEFACTORY_H
