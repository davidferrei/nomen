#ifndef CASEMANAGER_H
#define CASEMANAGER_H

#include <QString>
#include <QDebug>


struct CaseInfo {
    QString caseId;
    QString caseName;
    QString createdDate;
    QString updatedDate;
    QString notes;
    int caseState;
    QString casePath;
};


class CaseManager
{
public:
    CaseManager();

    bool createCase(const CaseInfo& info);
    bool openCase(const QString& casePath);

    QList<CaseInfo> loadPreviousCases() const;
    void saveCasePath(const QString& casePath);

    const CaseInfo& getCurrentCase() const;

    //TODO: Implement a Case Validation Func that Checks if Cases are Valid (DBs exist, JSON Valid, etc...)
    //TODO: Delete Case Func

private:
    CaseInfo currentCase;
    QStringList previousCasesPathsList;
};

#endif // CASEMANAGER_H
