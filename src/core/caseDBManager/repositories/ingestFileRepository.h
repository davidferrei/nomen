#ifndef INGESTFILEREPOSITORY_H
#define INGESTFILEREPOSITORY_H

#include <ibpp/core/ibpp.h>
#include "data/ingest/ingestFileData.h"

class IngestFileRepository
{
public:
    explicit IngestFileRepository(IBPP::Database db);

    bool insert(IngestFileData &data);

private:
    IBPP::Database fb_db;
};

#endif // INGESTFILEREPOSITORY_H
