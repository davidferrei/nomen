#ifndef INGESTFILEDATA_H
#define INGESTFILEDATA_H

#include <QString>

struct IngestFileData {
    qint64 id = 0;
    qint64 datasourceId = -1;
    qint64 partitionId = -1;
    qint64 filesystemId = -1;
    QString lmdbPath;
    QString lmdbPathHash;
    QString notes;
};

#endif // INGESTFILEDATA_H
