#ifndef CASEDBTHREADINSTANCE_H
#define CASEDBTHREADINSTANCE_H

#include <ibpp/core/ibpp.h>

#include <QString>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QThread>


#include <filesystem>

#include "core/caseDBManager/repositories/investigatorRepository.h"
#include "core/caseDBManager/repositories/organizationRepository.h"
#include "core/caseDBManager/repositories/datasourceRepository.h"
#include "core/caseDBManager/repositories/caseinformationRepository.h"
#include "core/caseDBManager/repositories/ingestJobHistoryRepository.h"
#include "core/caseDBManager/repositories/ingestResultItemRepository.h"

//TODO: Create caseDBEngine Class use in both CaseDBManager and CaseDBThreadInstance

class CaseDBThreadInstance
{
public:
    explicit CaseDBThreadInstance(const QString& dbPath);
    ~CaseDBThreadInstance();

    bool openDatabase();
    void closeDatabase();

    IBPP::Database database() const;
    bool databaseExists() const;

    InvestigatorRepository investigatorRepo() { return InvestigatorRepository(fb_db); };
    OrganizationRepository organizationRepo() { return OrganizationRepository(fb_db); };
    DataSourceRepository dataSourceRepo() { return DataSourceRepository(fb_db); };
    CaseInformationRepository caseInfoRepo() { return CaseInformationRepository(fb_db); };
    IngestJobHistoryRepository ingestJobsRepo() { return IngestJobHistoryRepository(fb_db); };
    IngestResultItemRepository ingestResultItemsRepo() { return IngestResultItemRepository(fb_db); };

private:
    IBPP::Database fb_db;
    QString dbPath;
};

#endif // CASEDBTHREADINSTANCE_H
