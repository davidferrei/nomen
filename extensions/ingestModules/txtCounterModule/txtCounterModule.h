#ifndef TXTCOUNTERMODULE_H
#define TXTCOUNTERMODULE_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QJsonObject>
#include <cctype>

#include "core/extensions/ingestModuleInterface.h"
#include "core/extensions/tskIngestModuleInterface.h"

#include "core/ingest/repo/ingestResultDBInstance.h"


class TxtCounterModule : public tskIngestModuleInterface
{
    Q_OBJECT
public:
    explicit TxtCounterModule(const QJsonObject &metadata = QJsonObject(), QObject *parent = nullptr) : tskIngestModuleInterface(metadata, parent) {}
    ~TxtCounterModule() override = default;

    void init() override;
    QString getModuleName() const override { return name(); }

    void processFile(const FileContext &fileContext) override;

    void finishWalk() override;

private:
    uint64_t filesCount = 0;
    uint64_t dirsCount = 0;
    uint64_t txtFilesCount = 0;
    qint64 resultFolderId = -1;

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
