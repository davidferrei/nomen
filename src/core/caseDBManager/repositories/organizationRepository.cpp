#include "organizationRepository.h"


OrganizationRepository::OrganizationRepository(IBPP::Database casedb) : fb_db(casedb) {}

//TODO: Make These Safe

bool OrganizationRepository::insert(const OrganizationData& org)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "INSERT INTO ORGANIZATION (NAME, EMAIL, NOTES, PHONENUMBER) "
        "VALUES (?, ?, ?, ?)"
        );

    st->Set(1, org.name.toStdString());
    st->Set(2, org.email.toStdString());
    st->Set(3, org.notes.toStdString());
    st->Set(4, org.phoneNumber.toStdString());

    st->Execute();
    tr->Commit();

    return true;
}


bool OrganizationRepository::update(int id, const OrganizationData& org)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare(
        "UPDATE ORGANIZATION "
        "SET NAME = ?, EMAIL = ?, NOTES = ?, PHONENUMBER = ? "
        "WHERE ORGANIZATIONID = ?"
        );

    st->Set(1, org.name.toStdString());
    st->Set(2, org.email.toStdString());
    st->Set(3, org.notes.toStdString());
    st->Set(4, org.phoneNumber.toStdString());
    st->Set(5, id);

    st->Execute();
    tr->Commit();

    return true;
}

bool OrganizationRepository::remove(int id)
{
    IBPP::Transaction tr = IBPP::TransactionFactory(fb_db);
    tr->Start();

    IBPP::Statement st = IBPP::StatementFactory(fb_db, tr);
    st->Prepare("DELETE FROM ORGANIZATION WHERE ORGANIZATIONID = ?");
    st->Set(1, id);

    st->Execute();
    tr->Commit();

    return true;
}
