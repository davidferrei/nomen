#ifndef CASEMANAGER_H
#define CASEMANAGER_H

#include <ibpp/core/ibpp.h>


#include <QString>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QSettings>


#include "core/caseDBManager/casedbmanager.h"
#include "core/logging/logger.h"


#include "data/case/caseInformationData.h"

//TODO: Implement a Case Validation Func that Checks if Cases are Valid (DBs exist, JSON Valid, etc...)
//TODO: Delete Case Func
//TODO: Get Case DB

class CaseManager
{
public:
    CaseManager();

    //Case Logic
    bool createCaseInfo(const CaseInfo& info);
    bool loadCaseInfo(const QString& casePath);
    bool openCase(const QString& casePath);

    //Previous Case History
    QList<CaseInfo> loadPreviousCases() const;
    void saveCasePath(const QString& casePath);

    //Getter/Setters
    CaseInfo& getCurrentCase();
    bool markCurrentCaseEnvironmentInitialized();
    
    //Access DB
    bool saveInvestigator(const InvestigatorData& inv);
    bool saveOrganization(const OrganizationData& org);
    bool saveDataSource(const DataSourceData& ds);
    bool saveCaseInfo(const CaseInfo& c);

    QList<DataSourceData> getDataSources() const;

    QList<IngestJobHistoryData> getIngestJobs() const;
    QList<IngestResultItemData> getIngestResultItemsForJob(qint64 jobId) const;
    QList<IngestResultItemData> getIngestResultChildren(qint64 parentId) const;


private:
    bool initEnvironment();

private:
    CaseInfo currentCase;
    QStringList previousCasesPathsList;

    CaseDBManager* currentCaseDB = nullptr;
};

#endif // CASEMANAGER_H
