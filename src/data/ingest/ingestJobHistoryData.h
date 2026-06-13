#ifndef INGESTJOBHISTORYDATA_H
#define INGESTJOBHISTORYDATA_H

#include <QString>
#include <QDateTime>

struct IngestJobHistoryData {
    qint64 id = 0;
    QString moduleName;
    QString moduleVersion;
    QDateTime startTime;
    QDateTime endTime;
    int status = 0; // 0=Pending,1=Running,2=OK,3=NOK
    QString log;
    QString settingsJson;
};

#endif // INGESTJOBHISTORYDATA_H
