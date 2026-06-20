#include "yaraXScanner.h"

YaraXScanner::YaraXScanner() : yrxRules(nullptr), yrxScanner(nullptr), filesScannedCount(0), matchesFoundCount(0) {}

YaraXScanner::~YaraXScanner()
{
    cleanup();
}

//############################################################################################
// Callbacks

static void metadata_callback(const YRX_METADATA *ruleMetadata, void *user_data)
{
    QStringList *ruleList = static_cast<QStringList*>(user_data);

    QString metadataValue;
    switch (ruleMetadata->value_type) 
    {
        case YRX_I64:
            metadataValue = QString::number(ruleMetadata->value.i64); 
            break;
        case YRX_F64:
            metadataValue = QString::number(ruleMetadata->value.f64); 
            break;
        case YRX_BOOLEAN: 
            metadataValue = ruleMetadata->value.boolean ? "true" : "false"; 
            break;
        case YRX_STRING:
            metadataValue = QString::fromUtf8(ruleMetadata->value.string);
            break;
        default:
            metadataValue = "<bytes>"; 
            break;
    }

    ruleList->append(QString("%1=%2").arg(ruleMetadata->identifier, metadataValue));
}

static void match_offset_callback(const YRX_MATCH *yrxMatch, void *user_data)
{
    QString *matchOffsetsString = static_cast<QString*>(user_data);
    if (!matchOffsetsString->isEmpty()) 
        matchOffsetsString->append(", ");
        
    matchOffsetsString->append(QString("offset:%1 len:%2").arg(yrxMatch->offset).arg(yrxMatch->length));
}

static void pattern_callback(const YRX_PATTERN *yrxPattern, void *user_data)
{
    QStringList *patternsList = static_cast<QStringList*>(user_data);

    const uint8_t *patternIdentifierBytes;
    size_t patternIDLength; 
    yrx_pattern_identifier(yrxPattern, &patternIdentifierBytes, &patternIDLength);
    QString patternName = QString::fromUtf8(reinterpret_cast<const char*>(patternIdentifierBytes), static_cast<int>(patternIDLength));

    QString patternMatchOffsets;
    yrx_pattern_iter_matches(yrxPattern, match_offset_callback, &patternMatchOffsets);

    if (!patternMatchOffsets.isEmpty())
        patternsList->append(patternName + " [" + patternMatchOffsets + "]");
}

static void matching_rule_callback(const YRX_RULE *yrxRule, void *user_data)
{
    YaraXScanner *yrxScannerData = static_cast<YaraXScanner*>(user_data);
    yrxScannerData->incrementMatchesFoundCount();

    if (static_cast<int>(yrxScannerData->getStoredMatches().size()) >= 20) //Debug
        return;

    const uint8_t *ruleIDBytes;
    size_t ruleIDLength;
    yrx_rule_identifier(yrxRule, &ruleIDBytes, &ruleIDLength);
    QString ruleName = QString::fromUtf8(reinterpret_cast<const char*>(ruleIDBytes), static_cast<int>(ruleIDLength));

    const uint8_t *namespaceBytes;
    size_t namespaceLength;
    yrx_rule_namespace(yrxRule, &namespaceBytes, &namespaceLength);
    QString ruleNamespace = QString::fromUtf8(reinterpret_cast<const char*>(namespaceBytes), static_cast<int>(namespaceLength));

    QStringList ruleMetadataList;
    yrx_rule_iter_metadata(yrxRule, metadata_callback, &ruleMetadataList);

    QStringList rulePatternsList;
    yrx_rule_iter_patterns(yrxRule, pattern_callback, &rulePatternsList);

    YaraXMatchResultData yrxMatchResultdata
    {
        yrxScannerData->getCurrentDatasourceId(),
        yrxScannerData->getCurrentPartitionId(),
        yrxScannerData->getCurrentFilesystemId(),
        yrxScannerData->getCurrentTskLmdbPath(),
        yrxScannerData->getCurrentFilePath(),
        ruleName,
        ruleNamespace,
        ruleMetadataList,
        rulePatternsList
    };
    const_cast<std::vector<YaraXMatchResultData>&>(yrxScannerData->getStoredMatches()).push_back(yrxMatchResultdata);

    qInfo() << "[YaraX] Match #" << yrxScannerData->getMatchesFoundCount()
            << "| File:" << yrxMatchResultdata.filePath
            << "| Rule:" << yrxMatchResultdata.ruleName
            << "| NS:" << yrxMatchResultdata.ruleNamespace;

}

//############################################################################################
// Functions

bool YaraXScanner::initialize()
{
    qInfo() << "[YaraX] Initializing Scanner...";
    resetCounters();

    QString rulesDir;
    if (!findYaraRulesDirectory(rulesDir)) 
    {
        qWarning() << "[YaraX] Could not Find YARA-X-Rules Directory";
        return false;
    }
    qInfo() << "[YaraX] Found Rules Directory:" << rulesDir;

    QByteArray rulesContentBA;
    if (!loadRulesFromDirectory(rulesDir, rulesContentBA)) 
    {
        qWarning() << "[YaraX] Failed to Load Rules";
        return false;
    }

    int compileResult = yrx_compile(rulesContentBA.constData(), &yrxRules);
    if (compileResult != YRX_SUCCESS) 
    {
        const char *err = yrx_last_error();
        qWarning() << "[YaraX] Failed to Compile Rules:" << (err ? err : "Unknown error");
        yrxRules = nullptr;

        return false;
    }

    qInfo() << "[YaraX] Rules Compiled Successfully. Total Rules:" << yrx_rules_count(yrxRules);

    int scannerResult = yrx_scanner_create(yrxRules, &yrxScanner);
    if (scannerResult != YRX_SUCCESS) 
    {
        const char *err = yrx_last_error();
        qWarning() << "[YaraX] Failed to Create Scanner:" << (err ? err : "Unknown error");
        yrxScanner = nullptr;
        yrx_rules_destroy(yrxRules);
        yrxRules = nullptr;

        return false;
    }

    yrx_scanner_on_matching_rule(yrxScanner, matching_rule_callback, this);

    qInfo() << "[YaraX] Scanner ready";
    return true;
}


bool YaraXScanner::scanBuffer(const uint8_t *buffer, size_t bufferSize)
{
    if (!yrxScanner) 
    {
        qWarning() << "[YaraX] Scanner is not Initialized.";
        return false;
    }

    int scanResult = yrx_scanner_scan(yrxScanner, buffer, bufferSize);

    if (scanResult != YRX_SUCCESS) 
    {
        const char *err = yrx_last_error();
        qWarning() << "[YaraX] Scan Failed:" << (err ? err : "Unknown error");

        return false;
    }

    return true;
}

bool YaraXScanner::findYaraRulesDirectory(QString &outPath)
{
    const QDir appDirPath(QCoreApplication::applicationDirPath());
    const QStringList extensionsSearchPaths = {
        appDirPath.filePath("plugins/YARA-X-Rules"),
        appDirPath.filePath("../plugins/YARA-X-Rules"),
        appDirPath.filePath("../extensions/ingestModules/YARA-X-Rules")
    };

    for (const QString &path : extensionsSearchPaths) 
    {
        QDir dir(path);
        if (dir.exists()) 
        {
            outPath = dir.absolutePath();
            return true;
        }
    }
    return false;
}

bool YaraXScanner::loadRulesFromDirectory(const QString &dirPath, QByteArray &outRules)
{
    QDir yaraRulesDir(dirPath);
    QStringList yarFiles = yaraRulesDir.entryList(QStringList() << "*.yar", QDir::Files);

    if (yarFiles.isEmpty()) 
    {
        qWarning() << "[YaraX] No .yar Files Found in" << dirPath;
        return false;
    }

    for (const QString &fileName : yarFiles) 
    {
        QFile file(yaraRulesDir.filePath(fileName));

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
        {
            qWarning() << "[YaraX] Failed to Open Rule File:" << fileName;
            continue;
        }

        outRules.append(file.readAll());
        outRules.append('\n');
        file.close();

        qInfo() << "[YaraX] Loaded Rule File:" << fileName;
    }

    return !outRules.isEmpty();
}

void YaraXScanner::resetCounters()
{
    filesScannedCount = 0;
    matchesFoundCount = 0;
}

void YaraXScanner::cleanup()
{
    if (yrxScanner) 
    {
        yrx_scanner_destroy(yrxScanner);
        yrxScanner = nullptr;
    }
    
    if (yrxRules) 
    {
        yrx_rules_destroy(yrxRules);
        yrxRules = nullptr;
    }
}
