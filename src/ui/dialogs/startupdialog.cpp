#include "startupdialog.h"
#include "ui_startupdialog.h"

startupDialog::startupDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::startupDialog)
{
    ui->setupUi(this);
}

startupDialog::~startupDialog()
{
    delete ui;
}

void startupDialog::on_pushButton_clicked()
{
    accept();
}

