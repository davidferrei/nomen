#include "casemanager.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

CaseManager::CaseManager() {}


bool CaseManager::createCase(const CaseInfo& info)
{
    if (info.caseId.isEmpty())
        return false;

    QString caseDirPath = info.casePath;
    QDir caseDir(info.casePath);

    if (!caseDir.mkpath("indexes")) return false;
    if (!caseDir.mkpath("case_db")) return false;
    if (!caseDir.mkpath("evidence")) return false;
    if (!caseDir.mkpath("logs")) return false;
    if (!caseDir.mkpath("reports")) return false;
    if (!caseDir.mkpath("exports")) return false;

    QJsonObject jObj;
    jObj["case_id"] = info.caseId;
    jObj["case_name"] = info.caseName;
    jObj["created_date"] = info.createdDate;
    jObj["updated_date"] = info.updatedDate;
    jObj["notes"] = info.notes;
    jObj["case_state"] = info.caseState;

    QFile file(caseDirPath + "/case_info.json");
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot Create Case File at:" << caseDirPath << "/case_info.json";
        return false;
    }

    file.write(QJsonDocument(jObj).toJson(QJsonDocument::Indented));
    file.close();

    qInfo() << "Case Created:" << caseDirPath;
    qDebug() << "###################################";
    qDebug() << "Case ID:" << info.caseId;
    qDebug() << "Case Name:" << info.caseName;
    qDebug() << "Created Date:" << info.createdDate;
    qDebug() << "Updated Date:" << info.updatedDate;
    qDebug() << "Case State:" << info.caseState;
    qDebug() << "Notes:" << info.notes;
    qDebug() << "###################################";
    return true;
}


bool CaseManager::openCase(const QString& casePath)
{
    QFile file(casePath + "/case_info.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open case file at:" << casePath << "/case_info.json";
        return false;
    }

    QJsonDocument jDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!jDoc.isObject())
    {
        qWarning() << "Case File is Invalid: " << casePath << "/case_info.json";
        return false;
    }

    QJsonObject jObj = jDoc.object();

    currentCase.caseId = jObj.value("case_id").toString();
    currentCase.caseName = jObj.value("case_name").toString();
    currentCase.createdDate = jObj.value("created_date").toString();
    currentCase.updatedDate = jObj.value("updated_date").toString();
    currentCase.notes = jObj.value("notes").toString();
    currentCase.caseState = jObj.value("case_state").toInt();
    currentCase.casePath = casePath;

    qInfo() << "Case Opened:" << currentCase.caseName;
    qDebug() << "###################################";
    qDebug() << "Case ID:" << currentCase.caseId;
    qDebug() << "Case Name:" << currentCase.caseName;
    qDebug() << "Created Date:" << currentCase.createdDate;
    qDebug() << "Updated Date:" << currentCase.updatedDate;
    qDebug() << "Case State:" << currentCase.caseState;
    qDebug() << "Notes:" << currentCase.notes;
    qDebug() << "Case Path:" << currentCase.casePath;
    qDebug() << "###################################";
    return true;
}

const CaseInfo& CaseManager::getCurrentCase() const
{
    return currentCase;
}

