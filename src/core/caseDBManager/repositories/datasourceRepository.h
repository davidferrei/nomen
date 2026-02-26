#ifndef DATASOURCEREPOSITORY_H
#define DATASOURCEREPOSITORY_H

#include <ibpp/core/ibpp.h>

#include "data/evidence/dataSourceData.h"


class DataSourceRepository
{
public:
    DataSourceRepository(IBPP::Database db);

    bool insert(const DataSourceData& ds);
    bool update(long long id, const DataSourceData& ds);
    bool remove(long long id);

private:
    IBPP::Database fb_db;

};

#endif // DATASOURCEREPOSITORY_H
