#ifndef ORGANIZATIONREPOSITORY_H
#define ORGANIZATIONREPOSITORY_H

#include <ibpp/core/ibpp.h>

#include "data/case/organizationData.h"


class OrganizationRepository
{
public:
    OrganizationRepository(IBPP::Database casedb);

    bool insert(const OrganizationData& organization);
    bool update(int id, const OrganizationData& organization);
    bool remove(int id);

private:
    IBPP::Database fb_db;

};

#endif // ORGANIZATIONREPOSITORY_H
