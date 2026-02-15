#ifndef INVESTIGATORREPOSITORY_H
#define INVESTIGATORREPOSITORY_H

#include <ibpp/core/ibpp.h>

#include "data/case/investigatorData.h"


class InvestigatorRepository
{
public:
    InvestigatorRepository(IBPP::Database casedb);

    bool insert(const InvestigatorData& investigator);
    bool update(int id, const InvestigatorData& investigator);
    bool remove(int id);

private:
    IBPP::Database fb_db;

};

#endif // INVESTIGATORREPOSITORY_H
