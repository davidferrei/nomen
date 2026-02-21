#include "casemanager.h"

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
    if (!caseDir.mkpath("logs/ingest")) return false;
    if (!caseDir.mkpath("logs/index")) return false;
    if (!caseDir.mkpath("reports")) return false;
    if (!caseDir.mkpath("exports")) return false;

    QJsonObject jObj;
    jObj["case_id"] = info.caseId;
    jObj["case_name"] = info.caseName;
    jObj["created_date"] = info.createdDate;
    jObj["updated_date"] = info.updatedDate;
    jObj["notes"] = info.notes;
    jObj["case_state"] = info.caseState;
    jObj["environment_initialized"] = false;

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
    qDebug() << "Environment Initialized:" << false;
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
    currentCase.isEnvInitialized = jObj.value("environment_initialized").toBool();
    currentCase.casePath = casePath;

    qInfo() << "Case Opened:" << currentCase.caseName;
    qDebug() << "###################################";
    qDebug() << "Case ID:" << currentCase.caseId;
    qDebug() << "Case Name:" << currentCase.caseName;
    qDebug() << "Created Date:" << currentCase.createdDate;
    qDebug() << "Updated Date:" << currentCase.updatedDate;
    qDebug() << "Case State:" << currentCase.caseState;
    qDebug() << "Case Initialized:" << currentCase.isEnvInitialized;
    qDebug() << "Notes:" << currentCase.notes;
    qDebug() << "Case Path:" << currentCase.casePath;
    qDebug() << "###################################";
    return true;
}

bool CaseManager::markCurrentCaseEnvironmentInitialized()
{
    if(currentCase.isEnvInitialized)
    {
        qWarning() << "Case is Already Initialized";
        return true;
    }

    QString caseInfoPath = currentCase.casePath + "/case_info.json";
    QFile file(caseInfoPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Cannot Open Case Info File For Read";
        return false;
    }

    QJsonDocument jDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!jDoc.isObject())
    {
        qCritical() << "Case Info Json Is Invalid";
        return false;
    }

    QJsonObject jObj = jDoc.object();
    jObj["environment_initialized"] = true;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << "Cannot Open Case Info File For Write";
        return false;
    }

    file.write(QJsonDocument(jObj).toJson(QJsonDocument::Indented));
    file.close();

    currentCase.isEnvInitialized = true;

    qInfo() << "Case Environment Marked Initialized";

    return true;
}

//###########################################################################################
// Case Init

bool CaseManager::initEnvironment()
{

    if (!currentCaseDB)
    {
        qCritical() << "Tying to Initialize Database Before Opening It";
        return false;
    }

    if (!currentCaseDB->createDatabase())
    {
        return false;
    }

    qInfo() << "FIREBIRD: Database Created";

    if (!currentCaseDB->createSchema())
    {
        qCritical() << "Failed To Create Database Schema";

        QFile dbFile(currentCase.casePath + "/case_db/case.fdb");
        if (dbFile.exists())
        {
            if (dbFile.remove())
                qWarning() << "Partial DB Deleted.";
            else
                qFatal() << "Failed to Delete Partial DB!";
        }

        return false;
    }

    return true;
}



bool CaseManager::openCase(const QString& casePath)
{

    if (!loadCaseInfo(casePath)) {
        qCritical() << "Failed to Load Case Info";
        return false;
    }

    delete currentCaseDB;
    currentCaseDB = new CaseDBManager(casePath);

    if (!loadCaseInfo(casePath)) {
        qCritical() << "Failed to Load Case Info";
        return false;
    }

    QString dateForFile = QDateTime::currentDateTime().toString("dd_MM_yyyy");
    QString logFilePath = currentCase.casePath + "/logs/nomen_" + dateForFile + ".log";

    setLogFilePath(logFilePath);

    qInfo() << "Logging Into:" << logFilePath;

    if (!currentCaseDB->databaseExists())
    {
        if (currentCase.isEnvInitialized)
        {
            qCritical() << "Case DB is not Present But Environment is Marked as Initialized";
            return false;
        }

        qInfo() << "Initilizing Case Environment";
        if (!initEnvironment())
        {
            qCritical() << "Create Database Failed";
            return false;
        }
    }

    if (!currentCaseDB->openDatabase()) {
        qCritical() << "Database Open Test Failed";
        return false;
    }

    qInfo() << "Case Opened Successfully:" << currentCase.caseName;
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
// Access DB

bool CaseManager::saveInvestigator(const InvestigatorData& inv)
{

    if (!currentCaseDB) {
        qCritical() << "No case database open!";
        return false;
    }

    return currentCaseDB->investigatorRepo().insert(inv);
}

bool CaseManager::saveOrganization(const OrganizationData& org)
{

    if (!currentCaseDB) {
        qCritical() << "No case database open!";
        return false;
    }

    return currentCaseDB->organizationRepo().insert(org);
}


//###########################################################################################
// Case Setter/Getter


CaseInfo& CaseManager::getCurrentCase()
{
    return currentCase;
}

