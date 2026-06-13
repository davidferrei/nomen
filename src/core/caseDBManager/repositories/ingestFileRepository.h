#ifndef INGESTFILEREPOSITORY_H
#define INGESTFILEREPOSITORY_H

#include <ibpp/core/ibpp.h>


#include "data/ingest/ingestFileData.h"

#include <QPair>
#include <QDebug>

class IngestFileRepository
{
public:
    explicit IngestFileRepository(IBPP::Database db);

    bool insert(IngestFileData &data);
    QPair<bool, IngestFileData> getById(qint64 id);

private:
    IBPP::Database fb_db;
};

#endif // INGESTFILEREPOSITORY_H
