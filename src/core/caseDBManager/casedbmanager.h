#ifndef CASEDBMANAGER_H
#define CASEDBMANAGER_H

#include <ibpp/core/ibpp.h>

#include<QString>

#include "core/caseDBManager/repositories/investigatorRepository.h"

class CaseDBManager
{
public:
    CaseDBManager(const QString& casePath);
    ~CaseDBManager();

    bool createDatabase();
    bool createSchema();
    bool openDatabase();
    void closeDatabase();

    IBPP::Database database() const;
    bool databaseExists() const;

    //Repos
    InvestigatorRepository investigatorRepo() { return InvestigatorRepository(fb_db); };
    //OrganizationRepository organizationRepo();

private:
    IBPP::Database fb_db;
    QString dbPath;


};

#endif // CASEDBMANAGER_H
