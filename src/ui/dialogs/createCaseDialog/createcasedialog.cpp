#include "createcasedialog.h"
#include "ui_createcasedialog.h"

CreateCaseDialog::CreateCaseDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateCaseDialog)
{
    ui->setupUi(this);
}

CreateCaseDialog::~CreateCaseDialog()
{
    delete ui;
}
