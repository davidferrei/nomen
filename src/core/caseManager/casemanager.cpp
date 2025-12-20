#include "casemanager.h"

#include <QString>

struct CaseInfo {
    QString caseId;
    QString caseName;
    QString createdDate;
    QString updatedDate;
    QString notes;
    int caseState;
    QString casePath;
};

CaseManager::CaseManager() {}


bool createCase(const CaseInfo& info);
bool openCase(const QString& casePath);
