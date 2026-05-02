#ifndef INGESTRESULTITEMDATA_H
#define INGESTRESULTITEMDATA_H

#include <QString>

struct IngestResultItemData {
    qint64 id = 0;
    qint64 jobId = 0;
    qint64 parentItemId = -1; // "-1 = Ingest Result Item"

    enum class ItemType : int {
        Folder = 0,
        File = 1,
        Artifact = 2,
        Text = 3,
        Report = 4
    };

    ItemType itemType = ItemType::Folder;
    QString itemName;

    qint64 datasourceId = -1;
    int partitionId = -1;
    int filesystemId = -1;
    QString lmdbPath;
};

#endif // INGESTRESULTITEMDATA_H
