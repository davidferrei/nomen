#ifndef CASEMANAGER_H
#define CASEMANAGER_H

#include <ibpp/core/ibpp.h>


#include <QString>
#include <QDebug>

#include "core/caseDBManager/casedbmanager.h"


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


class CaseManager
{
public:
    CaseManager();

    bool createCaseInfo(const CaseInfo& info);
    bool loadCaseInfo(const QString& casePath);

    bool openCase(const QString& casePath);

    QList<CaseInfo> loadPreviousCases() const;
    void saveCasePath(const QString& casePath);

    const CaseInfo& getCurrentCase() const;

    //TODO: Implement a Case Validation Func that Checks if Cases are Valid (DBs exist, JSON Valid, etc...)
    //TODO: Delete Case Func
    //TODO: Get Case DB

private:

    bool initEnvironment();
    bool markCaseEnvironmentInitialized();

private:
    CaseInfo currentCase;
    QStringList previousCasesPathsList;

    CaseDBManager* currentCaseDB = nullptr;
};

#endif // CASEMANAGER_H
