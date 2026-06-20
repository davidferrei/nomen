#ifndef YARAXMATCHRESULTDATA_H
#define YARAXMATCHRESULTDATA_H

#include <QString>
#include <QStringList>

struct YaraXMatchResultData
{
    qint64 datasourceId = 0;
    qint64 partitionId = 0;
    qint64 filesystemId = 0;
    QString LMDBPath;
    
    QString filePath;
    QString ruleName;
    QString ruleNamespace;
    QStringList metadataList;
    QStringList patternsList;
};

#endif // YARAXMATCHRESULTDATA_H
