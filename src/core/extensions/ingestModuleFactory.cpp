#include "core/extensions/ingestModuleFactory.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::StringLiterals;

IngestModuleFactory::IngestModuleFactory()
{
    loadModulePlugins();
}

IngestModuleFactory &IngestModuleFactory::instance()
{
    static IngestModuleFactory singleton;
    return singleton;
}

IngestModuleFactory::~IngestModuleFactory()
{
    unloadModulePlugins();
}

//############################################################################################
// Functions


void IngestModuleFactory::loadModulePlugins()
{
    //FIXME: Handle Modules With Same Name (ID System?)

    if (!modulesHashTable.isEmpty())
        return;

    const QObjectList &staticPlugins = QPluginLoader::staticInstances();
    for (auto *plugin : staticPlugins)
        addModule(plugin, QJsonObject());

    const QDir appDir(QCoreApplication::applicationDirPath());

    QStringList candidates;
    candidates << appDir.filePath("../plugins");
    candidates << appDir.filePath("plugins"); 

    for (const QString &cand : candidates) 
    {
        QDir pluginsDir(cand);
        if (!pluginsDir.exists()) 
        {
            qWarning() << "[IngestModuleFactory] Plugin Dir Not Found:" << pluginsDir.absolutePath();
            continue;
        }

        qDebug() << "[IngestModuleFactory] Scanning Plugin Dir:" << pluginsDir.absolutePath();

        QStringList nameFilters;
#if defined(Q_OS_WIN)
        nameFilters << "*.dll";
#elif defined(Q_OS_MAC)
        nameFilters << "*.dylib" << "*.so"; //Untested
#else
        nameFilters << "*.so";
#endif
        const auto entryList = pluginsDir.entryList(nameFilters, QDir::Files);

        for (const QString &fileName : entryList) 
        {
            const QString absPath = pluginsDir.absoluteFilePath(fileName);
            QPluginLoader loader(absPath);
            const QJsonObject pluginInfo = loader.metaData();
            const QJsonObject pluginMetadata = pluginInfo.value("MetaData").toObject();
            if (!pluginInfo.isEmpty()) 
            {
                qDebug() << "[IngestModuleFactory] Plugin Metadata:" << QJsonDocument(pluginMetadata).toJson(QJsonDocument::Compact);
            }
            else 
            {
                qWarning() << "[IngestModuleFactory] No Metadata Found for Plugin:" << fileName;
            }

            QObject *plugin = loader.instance();
            if (plugin) 
            {
                qDebug() << "[IngestModuleFactory] Loaded Plugin:" << fileName;
                addModule(plugin, pluginMetadata);
            } else 
            {
                qWarning() << "[IngestModuleFactory] Plugin Load Failed:" << loader.errorString();
            }
        }
    }
}

void IngestModuleFactory::unloadModulePlugins()
{
    qDeleteAll(modulesHashTable.values());
    modulesHashTable.clear();
}



void IngestModuleFactory::addModule(QObject *moduleObject, const QJsonObject &pluginMetadata)
{
    auto *interface = qobject_cast<IngestModuleInterface *>(moduleObject);
    if (!interface)
    {
        qWarning() << "[IngestModuleFactory] Object Does not implement IngestModuleInterface:" << moduleObject;
        return;
    }
        
    AbstractIngestModule *pluginInstance = interface->module(pluginMetadata);
    if (!pluginInstance)
    {
        qWarning() << "[IngestModuleFactory] Failed to Create Module Instance for Plugin:" << interface->moduleName();
        return;
    }
    modulesHashTable.insert(interface->moduleName(), pluginInstance);
}

QStringList IngestModuleFactory::moduleNamesList() const
{
    return modulesHashTable.keys();
}

IngestModuleFactory::ModuleList IngestModuleFactory::modulesList() const
{
    return modulesHashTable.values();
}

AbstractIngestModule *IngestModuleFactory::fetchModule(const QString &moduleName) const
{
    auto it = modulesHashTable.find(moduleName);
    if (it != modulesHashTable.end())
        return it.value();

    qWarning() << "[IngestModuleFactory] Module" << moduleName << "Not Loaded.";
    return nullptr;
}


QJsonObject IngestModuleFactory::getModuleMetadata(const QString &moduleName) const
{
    if (auto *module = fetchModule(moduleName)) 
    {
        const QJsonObject moduleMetadata = module->getMetadata();
        if (moduleMetadata.isEmpty()) 
        {
            qWarning() << "[IngestModuleFactory] Metadata Empty for Module:" << moduleName;
            return QJsonObject();
        }
        return moduleMetadata;
    }

    qWarning() << "[IngestModuleFactory] Module:" << moduleName << "Not Found";
    return QJsonObject();
}


