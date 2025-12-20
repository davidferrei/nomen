#include "startupdialog.h"
#include "ui_startupdialog.h"

startupDialog::startupDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::startupDialog)
{
    ui->setupUi(this);

    qDebug() << "Startup Dialog Opened";

    //Widgets Setup
    ui->table_case->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    populateCaseTable();

}

startupDialog::~startupDialog()
{
    delete ui;
}


//############################################################################################
//Funcs

void startupDialog::populateCaseTable()
{

    ui->table_case->setRowCount(10); //TODO: Make this Dinamic
    for (int i = 0; i < 9; ++i) {
        ui->table_case->setItem(i, 0, new QTableWidgetItem(QString("Case")));
        ui->table_case->setItem(i, 1, new QTableWidgetItem(QString("Date")));
        ui->table_case->setItem(i, 2, new QTableWidgetItem(QString("Date")));
        ui->table_case->setItem(i, 3, new QTableWidgetItem(QString("Notes")));
    }
}

//############################################################################################
//Signals

void startupDialog::on_table_case_cellDoubleClicked(int row, int column)
{
    //TODO: Start Case
}

void startupDialog::on_pushButton_clicked()
{
    accept();
}
