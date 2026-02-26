#ifndef CASEINFORMATIONREPOSITORY_H
#define CASEINFORMATIONREPOSITORY_H

#include <ibpp/core/ibpp.h>

#include <QDate>


#include "data/case/caseInformationData.h"


class CaseInformationRepository
{
public:
    CaseInformationRepository(IBPP::Database db);

    bool insert(const CaseInfo& c);
    bool update(const QString& caseId, const CaseInfo& c);
    bool remove(const QString& caseId);

private:
    IBPP::Database fb_db;


    IBPP::Timestamp toIBPPTimestamp(const QString& dateStr);
};

#endif // CASEINFORMATIONREPOSITORY_H
