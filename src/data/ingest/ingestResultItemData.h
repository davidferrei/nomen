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

    qint64 fileId = -1;
};

#endif // INGESTRESULTITEMDATA_H
