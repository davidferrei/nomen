#ifndef CASEDATABASESCHEMA_H
#define CASEDATABASESCHEMA_H

#include <ibpp/core/ibpp.h>


class CaseDatabaseSchema
{
public:
    CaseDatabaseSchema();

    static bool createSchema(IBPP::Database db);
};

#endif // CASEDATABASESCHEMA_H
