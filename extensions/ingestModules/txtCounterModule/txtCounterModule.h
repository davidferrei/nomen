#ifndef TXTCOUNTERMODULE_H
#define TXTCOUNTERMODULE_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QJsonObject>


#include "extensions/ingestModules/ingestModuleInterface.h"

class TxtCounterModule : public AbstractIngestModule
{
    Q_OBJECT
public:
    explicit TxtCounterModule(const QJsonObject &metadata = QJsonObject(), QObject *parent = nullptr) : AbstractIngestModule(metadata, parent) {}
    ~TxtCounterModule() override = default;

    void init() override {}
    QString getModuleName() const override { return name(); }

    void process();

};

class TxtCounterPlugin : public QObject, public IngestModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.nomen.IngestModuleInterface" FILE "txtCounter.json")
    Q_INTERFACES(IngestModuleInterface)

public:
    ~TxtCounterPlugin() override = default;
    AbstractIngestModule *module(const QJsonObject &metadata) const override { return new TxtCounterModule(metadata, nullptr); }
    QString moduleName() const override { return "TxtCounter"; }
};

#endif // TXTCOUNTERMODULE_H
