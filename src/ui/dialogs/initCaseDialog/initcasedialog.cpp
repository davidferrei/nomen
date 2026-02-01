#include "initcasedialog.h"
#include "ui_initcasedialog.h"

initCaseDialog::initCaseDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::initCaseDialog)
{
    ui->setupUi(this);

    qDebug() << "Initialize Case Dialog Opened";

}

initCaseDialog::~initCaseDialog()
{
    qDebug() << "Initialize Case Dialog Destroyed";
    delete ui;
}
