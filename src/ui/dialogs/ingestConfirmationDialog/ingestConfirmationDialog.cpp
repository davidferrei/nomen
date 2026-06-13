#include "ingestConfirmationDialog.h"
#include "ui_ingestConfirmationDialog.h"

ingestConfirmationDialog::ingestConfirmationDialog(const QStringList &modulesList, QWidget *parent) : QDialog(parent), ui(new Ui::ingestConfirmationDialog)
{
    ui->setupUi(this);
    
    for (const QString &module : modulesList) 
    {
        ui->listWidget->addItem(module);
    }
}

ingestConfirmationDialog::~ingestConfirmationDialog()
{
    delete ui;
}
