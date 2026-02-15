#include "casemanager.h"

#include "casedbmanager.h"

#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>

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

CaseManager::~CaseManager()
{
    closeDatabase();
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

bool CaseManager::markCaseEnvironmentInitialized()
{
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

    qInfo() << "Case Environment Marked Initialized";
    currentCase.isEnvInitialized = true;

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
    qDebug() << "FIREBIRD: Creating Case Database";

    if (currentCase.casePath.isEmpty()) {
        qCritical() << "No Case Path Set";
        return false;
    }

    const QString dbDirPath = currentCase.casePath + "/case_db";
    const QString dbFilePath = dbDirPath + "/case.fdb";

    QDir caseDbDir(dbDirPath);
    if (!caseDbDir.exists()) {
        if (!caseDbDir.mkpath(".")) {
            qCritical() << "Failed to Create case_db Directory:" << dbDirPath;
            return false;
        }
    }


    //TODO: Windows Fix
    setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
    setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
    setenv("FIREBIRD_LOCK", "/tmp/firebird_lock", 1);

    QDir lockDir("/tmp/firebird_lock"); //TODO: Windows Fix

    if (!lockDir.exists()) {
        if (!lockDir.mkpath(".")) {
            qCritical() << "Failed to Create Firebird Lock Directory:" << lockDir.absolutePath();
            return false;
        }
    }

    try {
        const std::string dbPath = dbFilePath.toStdString();

        IBPP::Database db = IBPP::DatabaseFactory(
            "",
            dbPath,
            "SYSDBA", //TODO: Make User Choose This
            "masterkey"
            );

        if (!std::filesystem::exists(dbPath)) {
            db->Create(3);
            db->Disconnect();
        } else {
            qInfo() << "FIREBIRD: Database Already exists, Skipping Creation";
        }
    }
    catch (const IBPP::Exception& e) {
        qCritical() << "FIREBIRD: Error While Creating Database:" << e.ErrorMessage();
        return false;
    }
    catch (const std::exception& e) {
        qCritical() << "Exception While Creating Database:" << e.what();
        return false;
    }
    catch (...) {
        qCritical() << "FIREBIRD: Unknown Error While Creating Database";
        return false;
    }

    return true;
}

bool CaseManager::initEnvironment()
{

    if (!createDatabase())
    {
        return false;
    }

    qInfo() << "FIREBIRD: Database Created";

    IBPP::Database db = IBPP::DatabaseFactory("", currentCase.casePath.toStdString() + "/case_db/case.fdb", "SYSDBA", "masterkey");

    if (!CaseDBManager::createSchema(db))
    {
        qCritical() << "Failed To Create Database Schema";
        return false;
    }

    // if (!markCaseEnvironmentInitialized())
    // {
    //     qCritical() << "Failed To Mark Case Environment Initialized";
    //     return false;
    // }

    return true;
}


bool CaseManager::openDatabase()
{
    qDebug() << "FIREBIRD: Opening Case Database";

    const QString dbFilePath = currentCase.casePath + "/case_db/case.fdb";

    if (!QFile::exists(dbFilePath)) {
        qCritical() << "Database File Does not Exist:" << dbFilePath;
        return false;
    }

    try {
        setenv("FIREBIRD", "/home/admin1/dev/qt6/Nomen/fb_embedded", 1);
        setenv("LD_LIBRARY_PATH", "/home/admin1/dev/qt6/Nomen/fb_embedded/lib", 1);
        setenv("FIREBIRD_LOCK", "/tmp/firebird_lock", 1);

        const std::string dbPath = dbFilePath.toStdString();

        currentCaseDB = IBPP::DatabaseFactory(
            "",
            dbPath,
            "SYSDBA", //TODO: Make User Choose This
            "masterkey"
            );

        currentCaseDB->Connect();
        qInfo() << "FIREBIRD: Database Connection Successful";

        IBPP::Transaction tr = IBPP::TransactionFactory(currentCaseDB);
        tr->Start();

        IBPP::Statement st = IBPP::StatementFactory(currentCaseDB, tr);
        st->Prepare(
            "SELECT "
            "CURRENT_USER, "
            "CURRENT_ROLE, "
            "CAST(CURRENT_TIMESTAMP AS VARCHAR(64))"
            "FROM RDB$DATABASE"
            );

        st->Execute();

        std::string user;
        std::string role;
        std::string firebirdTime;

        if (st->Fetch()) {
            st->Get(1, user);
            st->Get(2, role);
            st->Get(3, firebirdTime);

            qDebug() << "##########################################";
            qDebug() << "FIREBIRD Stats";
            qDebug() << "FIREBIRD: Connected User:" << user.c_str();
            qDebug() << "FIREBIRD: Role:" << role.c_str();
            qDebug() << "FIREBIRD: Server Time:" << firebirdTime.c_str();
            qDebug() << "##########################################";
        }

        tr->Commit();
        //currentCaseDB->Disconnect();

    }
    catch (const IBPP::Exception& e) {
        qCritical() << "FIREBIRD: Error While Opening Database:" << e.ErrorMessage();
        return false;
    }
    catch (const std::exception& e) {
        qCritical() << "Exception While Opening Database:" << e.what();
        return false;
    }
    catch (...) {
        qCritical() << "FIREBIRD: Unknown Error While Opening Database";
        return false;
    }

    return true;
}


void CaseManager::closeDatabase()
{
    if (currentCaseDB->Connected()) {
        qDebug() << "FIREBIRD: Closing Database Connection";
        try {
            currentCaseDB->Disconnect();
        }
        catch (...) {
            qWarning() << "FIREBIRD: Exception While Closing Case DB";
        }
    }

    currentCaseDB = nullptr;
}

bool CaseManager::openCase(const QString& casePath)
{

    if (!loadCaseInfo(casePath)) {
        qCritical() << "Failed to Load Case Info";
        return false;
    }

    if (!databaseExists())
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

    if (!openDatabase()) {
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
// Case Setter/Getter


const CaseInfo& CaseManager::getCurrentCase() const
{
    return currentCase;
}

