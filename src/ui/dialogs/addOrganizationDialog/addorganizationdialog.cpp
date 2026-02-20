#include "addorganizationdialog.h"
#include "ui_addorganizationdialog.h"

addOrganizationDialog::addOrganizationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addOrganizationDialog)
{
    qDebug() << "Add Investigator Dialog Opened";

    ui->setupUi(this);
}

addOrganizationDialog::~addOrganizationDialog()
{
    qDebug() << "Add Investigator Dialog Destroyed";

    delete ui;
}

void addOrganizationDialog::setOrganization(const OrganizationData &data)
{
    //TODO: Add Validations
    ui->le_name_org->setText(data.name);
    ui->le_email_org->setText(data.email);
    ui->le_phone_org->setText(data.phoneNumber);
    ui->te_notes_org->setPlainText(data.notes);
}

OrganizationData addOrganizationDialog::getOrganization() const
{
    OrganizationData data;
    data.name = ui->le_name_org->text();
    data.email = ui->le_email_org->text();
    data.phoneNumber = ui->le_phone_org->text();
    data.notes = ui->te_notes_org->toPlainText();
    return data;
}
