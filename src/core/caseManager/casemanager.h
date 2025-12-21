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

    const CaseInfo& getCurrentCase() const;

private:
    CaseInfo currentCase;
};

#endif // CASEMANAGER_H
