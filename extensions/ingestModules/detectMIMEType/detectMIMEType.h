#ifndef DETECTMIMETYPE_H
#define DETECTMIMETYPE_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QJsonObject>

#include "core/extensions/ingestModuleInterface.h"

class DetectMIMETypeModule : public AbstractIngestModule
{
    Q_OBJECT
public:
    explicit DetectMIMETypeModule(const QJsonObject &metadata = QJsonObject(), QObject *parent = nullptr) : AbstractIngestModule(metadata, parent) {}
    ~DetectMIMETypeModule() override = default;

    void init() override {}
    QString getModuleName() const override { return name(); }

    void process();
};

class DetectMIMETypePlugin : public QObject, public IngestModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.nomen.IngestModuleInterface" FILE "detectMIMEType.json")
    Q_INTERFACES(IngestModuleInterface)

public:
    ~DetectMIMETypePlugin() override = default;
    AbstractIngestModule *module(const QJsonObject &metadata) const override { return new DetectMIMETypeModule(metadata, nullptr); }
    QString moduleName() const override { return "DetectMIMEType"; }
};

#endif // DETECTMIMETYPE_H
