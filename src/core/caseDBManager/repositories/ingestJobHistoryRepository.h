#ifndef INGESTJOBHISTORYREPOSITORY_H
#define INGESTJOBHISTORYREPOSITORY_H

#include <ibpp/core/ibpp.h>

#include "data/ingest/ingestJobHistoryData.h"
#include "../timestampHelpers.h"


#include <QList>

class IngestJobHistoryRepository
{
public:
    explicit IngestJobHistoryRepository(IBPP::Database db);

    bool insert(IngestJobHistoryData &job);
    bool update(qint64 id, const IngestJobHistoryData &job);
    bool remove(qint64 id);

    bool appendLog(qint64 id, const QString &logLine);
    bool setStatus(qint64 id, int status);

    QList<IngestJobHistoryData> getAll();

private:
    IBPP::Database fb_db;
};

#endif // INGESTJOBHISTORYREPOSITORY_H
