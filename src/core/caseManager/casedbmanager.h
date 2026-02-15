#ifndef CASEDBMANAGER_H
#define CASEDBMANAGER_H

#include <ibpp/core/ibpp.h>


class CaseDBManager
{
public:
    CaseDBManager();

    static bool createSchema(IBPP::Database db);


};

#endif // CASEDBMANAGER_H
