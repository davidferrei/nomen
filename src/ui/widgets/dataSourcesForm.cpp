#include "dataSourcesForm.h"
#include "ui_dataSourcesForm.h"

DataSourcesForm::DataSourcesForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DataSourcesForm)
{
    ui->setupUi(this);
}

DataSourcesForm::~DataSourcesForm()
{
    delete ui;
}
