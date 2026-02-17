#include "investigatorRepository.h"


InvestigatorRepository::InvestigatorRepository(IBPP::Database casedb) : fb_db(casedb) {}

//TODO: Make These Safe

bool InvestigatorRepository::insert(const InvestigatorData& inv)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "INSERT INTO INVESTIGATOR (NAME, EMAIL, NOTES, PHONENUMBER) "
        "VALUES (?, ?, ?, ?)"
        );

    st->Set(1, inv.name.toStdString());
    st->Set(2, inv.email.toStdString());
    st->Set(3, inv.notes.toStdString());
    st->Set(4, inv.phoneNumber.toStdString());

    st->Execute();
    tr->Commit();

    return true;
}


bool InvestigatorRepository::update(int id, const InvestigatorData& inv)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "UPDATE INVESTIGATOR "
        "SET NAME = ?, EMAIL = ?, NOTES = ?, PHONENUMBER = ? "
        "WHERE INVESTIGATORID = ?"
        );

    st->Set(1, inv.name.toStdString());
    st->Set(2, inv.email.toStdString());
    st->Set(3, inv.notes.toStdString());
    st->Set(4, inv.phoneNumber.toStdString());
    st->Set(5, id);

    st->Execute();
    tr->Commit();

    return true;
}

bool InvestigatorRepository::remove(int id)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare("DELETE FROM INVESTIGATOR WHERE INVESTIGATORID = ?");
    st->Set(1, id);

    st->Execute();
    tr->Commit();

    return true;
}
