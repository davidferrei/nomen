#include "ingestConfirmationDialog.h"
#include "ui_ingestConfirmationDialog.h"

ingestConfirmationDialog::ingestConfirmationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ingestConfirmationDialog)
{
    ui->setupUi(this);
}

ingestConfirmationDialog::~ingestConfirmationDialog()
{
    delete ui;
}
