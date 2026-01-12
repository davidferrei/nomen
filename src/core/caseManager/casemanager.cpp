#include "casemanager.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QSettings>

CaseManager::CaseManager()
{
    QSettings settings("Nomen", "Nomen");
    previousCasesPathsList = settings.value("cases/paths").toStringList();
}

//###########################################################################################
// Case Info

bool CaseManager::createCaseInfo(const CaseInfo& info)
{
    if (info.caseId.isEmpty())
        return false;

    QString caseDirPath = info.casePath; //REFAC: Use caseDir for Paths as well
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

    saveCasePath(info.casePath);

    return true;
}


bool CaseManager::loadCaseInfo(const QString& casePath)
{
    QFile file(casePath + "/case_info.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot Open Case File At:" << casePath << "/case_info.json";
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

//###########################################################################################
// Case Init

bool CaseManager::databaseExists() const
{
    return QFile::exists(currentCase.casePath + "/case_db/case.fdb");
}

bool CaseManager::createDatabase()
{
    qDebug() << "Creating Case Database - TO BE IMPLEMENTED";
    return true;
}

bool CaseManager::initEnvironment()
{

    // if (!createDatabase())
    //     return false;


    return true;
}


bool CaseManager::openCase(const QString& casePath)
{
    if (!loadCaseInfo(casePath)) {
        qCritical() << "Failed to Load Case Info";
        return false;
    }

    if (!databaseExists())
    {
        qInfo() << "Database not Found - Initilizing Case Environment";
        if (!initEnvironment())
        {
            qCritical() << "Create Database Failed";
        }
    }

    //TODO: Open DB

    return true;
}


//###########################################################################################
// QSettings History

void CaseManager::saveCasePath(const QString& casePath)
{
    if (previousCasesPathsList.contains(casePath))
        return;

    previousCasesPathsList.append(casePath);

    QSettings settings("Nomen", "Nomen");
    settings.setValue("cases/paths", previousCasesPathsList);

    qDebug() << "New Case Path Stored With QSettings";
}

QList<CaseInfo> CaseManager::loadPreviousCases() const
{
    QList<CaseInfo> cases;

    for (const QString& path : previousCasesPathsList) {

        QFile file(path + "/case_info.json");
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Missing case_info.json in " << path;
            continue;
        }

        QJsonDocument jDoc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!jDoc.isObject())
            continue;

        QJsonObject jObj = jDoc.object();

        CaseInfo info;
        info.caseId = jObj.value("case_id").toString();
        info.caseName = jObj.value("case_name").toString();
        info.createdDate = jObj.value("created_date").toString();
        info.updatedDate = jObj.value("updated_date").toString();
        info.notes = jObj.value("notes").toString();
        info.caseState = jObj.value("case_state").toInt();
        info.casePath = path;

        cases.append(info);
    }

    return cases;
}

//###########################################################################################
// Case Setter/Getter


const CaseInfo& CaseManager::getCurrentCase() const
{
    return currentCase;
}

