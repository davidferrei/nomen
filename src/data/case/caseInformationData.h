#ifndef CASEINFORMATIONDATA_H
#define CASEINFORMATIONDATA_H

#include <QString>

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
#endif // CASEINFORMATIONDATA_H
