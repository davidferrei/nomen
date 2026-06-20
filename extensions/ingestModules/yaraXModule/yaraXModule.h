#ifndef YARAXMODULE_H
#define YARAXMODULE_H

#include <QObject>
#include <QJsonObject>
#include <vector>

#include "core/extensions/ingestModuleInterface.h"
#include "core/extensions/tskIngestModuleInterface.h"
#include "core/ingest/repo/ingestResultDBInstance.h"

#include "yaraXScanner.h"
#include "yaraXMatchResultData.h"


class YaraXModule : public tskIngestModuleInterface
{
    Q_OBJECT
public:
    explicit YaraXModule(const QJsonObject &metadata = QJsonObject(), QObject *parent = nullptr) : tskIngestModuleInterface(metadata, parent) {}
    ~YaraXModule() override = default;

    void init() override;
    QString getModuleName() const override { return name(); }

    void processFile(const FileContext &fileContext) override;
    void finishWalk() override;

private:
    YaraXScanner YaraScanner;
    std::vector<YaraXMatchResultData> yrxMatchResultsVector;
    qint64 resultsRootFolderId = -1;
};

class YaraXPlugin : public QObject, public IngestModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.nomen.IngestModuleInterface" FILE "yaraXModule.json")
    Q_INTERFACES(IngestModuleInterface)

public:
    ~YaraXPlugin() override = default;
    AbstractIngestModule *module(const QJsonObject &metadata) const override { return new YaraXModule(metadata, nullptr); }
    QString moduleName() const override { return "Yara-X"; }
};

#endif // YARAXMODULE_H
