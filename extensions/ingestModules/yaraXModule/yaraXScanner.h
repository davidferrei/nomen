#ifndef YARAXSCANNER_H
#define YARAXSCANNER_H

#include <QCoreApplication>
#include <QString>
#include <cstdint>
#include <vector>
#include <QDir>
#include <QFile>
#include <QDebug>

#include "yaraXMatchResultData.h"


extern "C" {
#include <yara_x.h>
}

class YaraXScanner
{
public:
    YaraXScanner();
    ~YaraXScanner();

    bool initialize();

    bool scanBuffer(const uint8_t *buffer, size_t bufferSize);

    bool isReady() const { return yrxScanner != nullptr; }
    uint64_t getFilesScannedCount() const { return filesScannedCount; }
    uint64_t getMatchesFoundCount() const { return matchesFoundCount; }

    void setCurrentFilePath(const QString &path) { currentFilePath = path; }
    const QString &getCurrentFilePath() const { return currentFilePath; }

    void setCurrentFileTskMetadata(qint64 dsId, qint64 partId, qint64 fsId, const QString &lmdbPath)
    {
        currentDatasourceId = dsId;
        currentPartitionId = partId;
        currentFilesystemId = fsId;
        currentTskLmdbPath = lmdbPath;
    }

    qint64 getCurrentDatasourceId() const { return currentDatasourceId; }
    qint64 getCurrentPartitionId() const { return currentPartitionId; }
    qint64 getCurrentFilesystemId() const { return currentFilesystemId; }
    const QString &getCurrentTskLmdbPath() const { return currentTskLmdbPath; }

    void resetCounters();
    void incrementFilesScannedCount() { filesScannedCount++; }
    void incrementMatchesFoundCount() { matchesFoundCount++; }

    void cleanup();
    
    const std::vector<YaraXMatchResultData>& getStoredMatches() const { return storedMatches; }

private:
    YRX_RULES *yrxRules;
    YRX_SCANNER *yrxScanner;
    uint64_t filesScannedCount;
    uint64_t matchesFoundCount;
    QString currentFilePath;
    qint64 currentDatasourceId = 0;
    qint64 currentPartitionId = 0;
    qint64 currentFilesystemId = 0;
    QString currentTskLmdbPath;
    std::vector<YaraXMatchResultData> storedMatches;

    static bool findYaraRulesDirectory(QString &outPath);
    static bool loadRulesFromDirectory(const QString &dirPath, QByteArray &outRules);
};

#endif // YARAXSCANNER_H
