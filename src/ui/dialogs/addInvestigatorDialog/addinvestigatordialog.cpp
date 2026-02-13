#include "addinvestigatordialog.h"
#include "ui_addinvestigatordialog.h"

addInvestigatorDialog::addInvestigatorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addInvestigatorDialog)
{
    ui->setupUi(this);

    qDebug() << "Add Investigator Dialog Opened";
}

addInvestigatorDialog::~addInvestigatorDialog()
{
    qDebug() << "Add Investigator Dialog Destroyed";
    delete ui;
}

//###############################################################################



void addInvestigatorDialog::setInvestigator(const InvestigatorData &data)
{
    //TODO: Add Validations
    ui->le_name->setText(data.name);
    ui->le_email->setText(data.email);
    ui->le_phone->setText(data.phoneNumber);
    ui->te_notes->setPlainText(data.notes);
}

InvestigatorData addInvestigatorDialog::getInvestigator() const
{
    InvestigatorData data;
    data.name = ui->le_name->text();
    data.email = ui->le_email->text();
    data.notes = ui->te_notes->toPlainText();
    data.phoneNumber = ui->le_phone->text();

    return data;
}
