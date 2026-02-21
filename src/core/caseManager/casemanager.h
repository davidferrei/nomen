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


struct CaseInfo {
    QString caseId;
    QString caseName;
    QString createdDate;
    QString updatedDate;
    QString notes;
    int caseState;
    bool isEnvInitialized;
    QString casePath;
};


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
    bool saveOrganization(const OrganizationData& inv);

private:
    bool initEnvironment();

private:
    CaseInfo currentCase;
    QStringList previousCasesPathsList;

    CaseDBManager* currentCaseDB = nullptr;
};

#endif // CASEMANAGER_H
