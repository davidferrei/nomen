#include "addinvestigatordialog.h"
#include "ui_addinvestigatordialog.h"

addInvestigatorDialog::addInvestigatorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addInvestigatorDialog)
{
    ui->setupUi(this);
}

addInvestigatorDialog::~addInvestigatorDialog()
{
    delete ui;
}
