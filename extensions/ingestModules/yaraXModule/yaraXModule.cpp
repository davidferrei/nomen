#include "yaraXModule.h"

void YaraXModule::init()
{
    if (!YaraScanner.initialize()) 
    {
        throw std::runtime_error("[YaraX] Failed to Initialize the Scanner");
    }

    yrxMatchResultsVector.clear();
    
    if (ingestResultDB)
    {
        resultsRootFolderId = ingestResultDB->createFolderItem(getModuleName());
    }

    qInfo() << "[YaraX] Module Initialized";

}

void YaraXModule::processFile(const FileContext &fileContext)
{

    if (!YaraScanner.isReady()) 
    {
        throw std::runtime_error("[YaraX] Scanner is not Ready");
    }

    YaraScanner.incrementFilesScannedCount();

    if (!fileContext.hasMetadata || !fileContext.tsk_metadata || 
        fileContext.tsk_metadata->type != TSK_FS_META_TYPE_REG) 
    {
        return;
    }


    if (fileContext.tsk_metadata->size == 0 || fileContext.tsk_metadata->size > 500ULL * 1024 * 1024) 
    {
        return;
    }

    std::vector<uint8_t> fileSizeBuffer(fileContext.tsk_metadata->size);
    ssize_t fileBytesRead = fileContext.readFileContent(0, reinterpret_cast<char*>(fileSizeBuffer.data()), fileContext.tsk_metadata->size);
    
    if (fileBytesRead <= 0) 
    {
        qWarning() << "[YaraX] Failed to Read File:" << fileContext.filePath;
        return;
    }

    YaraScanner.setCurrentFilePath(fileContext.filePath);

    QString tskLmdbPath = QString("%1:%2:%3").arg(currentPartitionId).arg(currentFsIndex).arg(fileContext.filePath);
    YaraScanner.setCurrentFileTskMetadata(1, currentPartitionId, currentFsIndex, tskLmdbPath);

    if (!YaraScanner.scanBuffer(fileSizeBuffer.data(), fileBytesRead)) 
    {
        qWarning() << "[YaraX] Scan Failed for File:" << fileContext.filePath;
        return;
    }

    // if (YaraScanner.getFilesScannedCount() % 100000 == 0) 
    // {
    //     qInfo() << "[YaraX] Scanned" << YaraScanner.getFilesScannedCount() << "Files...";
    // }
}

void YaraXModule::finishWalk()
{
    qInfo() << "[YaraX] Module Finished";
    qInfo() << "[YaraX] Stats - "
            << "Files Scanned:" << YaraScanner.getFilesScannedCount()
            << "| Matches Found:" << YaraScanner.getMatchesFoundCount();

    if (ingestResultDB && resultsRootFolderId >= 0)
    {
        const auto& storedMatches = YaraScanner.getStoredMatches();
        qInfo() << "[YaraX] Storing" << storedMatches.size() << "Matches to DB";

        for (const auto& match : storedMatches)
        {
            qint64 matchFolderId = ingestResultDB->createFolderItem(resultsRootFolderId, QString("Match: %1 (%2)").arg(match.ruleName, match.filePath));

            if (matchFolderId < 0) 
            {
                qWarning() << "[YaraX] Failed to Create Folder for Match:" << match.ruleName;
                continue;
            }

            qint64 fileItemId = ingestResultDB->createFileItem(
                match.datasourceId,
                match.partitionId,
                match.filesystemId,
                match.LMDBPath,
                QString("Matched by Rule: %1").arg(match.ruleName),
                matchFolderId
            );

            if (fileItemId < 0)
                qWarning() << "[YaraX] Failed to Create File Item for:" << match.filePath;

            ingestResultDB->createTextItem(matchFolderId, "Rule", match.ruleName);
            ingestResultDB->createTextItem(matchFolderId, "Namespace", match.ruleNamespace);
            
            if (!match.metadataList.isEmpty())
                ingestResultDB->createTextItem(matchFolderId, "Metadata", match.metadataList.join(","));
            
            if (!match.patternsList.isEmpty())
                ingestResultDB->createTextItem(matchFolderId, "Patterns", match.patternsList.join(";"));
        }

    }

    YaraScanner.cleanup();
}

