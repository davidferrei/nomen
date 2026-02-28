#include "caseinformationRepository.h"

CaseInformationRepository::CaseInformationRepository(IBPP::Database db) : fb_db(db) {}

bool CaseInformationRepository::insert(const CaseInfo& c)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "INSERT INTO CaseTable "
        "(CASEID, NAME, CREATIONDATE, MODIFIEDDATE, NOTES, STATE, FILESAVEPATH) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)"
        );

    st->Set(1, c.caseId.toStdString());
    st->Set(2, c.caseName.toStdString());
    st->Set(3, toIBPPTimestamp(c.createdDate));
    st->Set(4, toIBPPTimestamp(c.updatedDate));
    st->Set(5, c.notes.toStdString());
    st->Set(6, static_cast<int16_t>(c.caseState));
    st->Set(7, c.casePath.toStdString());

    st->Execute();
    tr->Commit();

    return true;
}

bool CaseInformationRepository::update(const QString& caseId, const CaseInfo& c)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "UPDATE CaseTable "
        "SET NAME = ?, CREATIONDATE = ?, MODIFIEDDATE = ?, NOTES = ?, STATE = ?, FILESAVEPATH = ? "
        "WHERE CASEID = ?"
        );

    st->Set(1, c.caseName.toStdString());
    st->Set(2, toIBPPTimestamp(c.createdDate));
    st->Set(3, toIBPPTimestamp(c.updatedDate));
    st->Set(4, c.notes.toStdString());
    st->Set(5, static_cast<int16_t>(c.caseState));
    st->Set(6, c.casePath.toStdString());
    st->Set(7, c.caseId.toStdString());

    st->Execute();
    tr->Commit();

    return true;
}

bool CaseInformationRepository::remove(const QString& caseId)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare("DELETE FROM CASES WHERE CASEID = ?");
    st->Set(1, caseId.toStdString());

    st->Execute();
    tr->Commit();

    return true;
}

//TODO: Make this usable to other Repos

IBPP::Timestamp CaseInformationRepository::toIBPPTimestamp(const QString& dateStr)
{
    QDateTime dt = QDateTime::fromString(dateStr, Qt::ISODate);

    if (!dt.isValid()) {
        dt = QDateTime::currentDateTime();
    }

    QDate d = dt.date();
    QTime t = dt.time();

    return IBPP::Timestamp(d.year(), d.month(), d.day(),
                           t.hour(), t.minute(), t.second(),
                           t.msec() * 10);
}
