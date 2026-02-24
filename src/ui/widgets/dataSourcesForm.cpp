#include "dataSourcesForm.h"
#include "ui_dataSourcesForm.h"

DataSourcesForm::DataSourcesForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DataSourcesForm)
{
    ui->setupUi(this);

    setupDataSourceTable();
}

DataSourcesForm::~DataSourcesForm()
{
    delete ui;
}

//#####################################################################################
// Functions


void DataSourcesForm::setupDataSourceTable()
{
    ui->table_DS->setColumnCount(4);

    ui->table_DS->horizontalHeader()->setVisible(false);
    ui->table_DS->verticalHeader()->setVisible(false);

    ui->table_DS->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->table_DS->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->table_DS->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->table_DS->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    ui->table_DS->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_DS->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void DataSourcesForm::on_bttn_create_DS_clicked()
{
//     addDataSourceDialog dlg(this);

//     if (dlg.exec() == QDialog::Accepted)
//     {
//         DataSourceData inv = dlg.getDataSource();
//         dataSources.append(inv);
//         addDataSourceRow(inv, dataSources.size() - 1);
//     }
}

