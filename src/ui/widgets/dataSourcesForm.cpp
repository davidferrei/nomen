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
    ui->table_DS->clear();
    ui->table_DS->setRowCount(0);
    ui->table_DS->setColumnCount(5);

    QStringList headers;
    headers << "ID/Name" << "Time Zone" << "File Path" << "" << "";
    ui->table_DS->setHorizontalHeaderLabels(headers);

    ui->table_DS->verticalHeader()->setVisible(false);

    ui->table_DS->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->table_DS->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->table_DS->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->table_DS->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->table_DS->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    ui->table_DS->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_DS->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void DataSourcesForm::addDataSourceRow(const DataSourceData& ds, int index)
{
    int row = ui->table_DS->rowCount();
    ui->table_DS->insertRow(row);

    ui->table_DS->setItem(row, 0, new QTableWidgetItem(QString::number(index + 1)));
    ui->table_DS->setItem(row, 1, new QTableWidgetItem(ds.timeZone));
    ui->table_DS->setItem(row, 2, new QTableWidgetItem(ds.filePath));

    QPushButton* editBtn = new QPushButton();
    ui->table_DS->setCellWidget(row, 3, editBtn);

    connect(editBtn, &QPushButton::clicked, [this, editBtn]() {
        int row = ui->table_DS->indexAt(editBtn->pos()).row();
        editDataSource(row);
    });

    QPushButton* removeBtn = new QPushButton();
    ui->table_DS->setCellWidget(row, 4, removeBtn);

    connect(removeBtn, &QPushButton::clicked, [this, removeBtn]() {
        int row = ui->table_DS->indexAt(removeBtn->pos()).row();
        removeDataSource(row);
    });

    editBtn->setIcon(QIcon::fromTheme("document-edit", QIcon(":/icons/pencil.png")));
    editBtn->setToolTip("Edit Data Source");

    removeBtn->setIcon(QIcon::fromTheme("edit-delete", QIcon(":/icons/trash.png")));
    removeBtn->setToolTip("Remove Data Source");
}
void DataSourcesForm::refreshDataSourceTable()
{
    ui->table_DS->setRowCount(0);

    for (int i = 0; i < dataSources.size(); ++i)
        addDataSourceRow(dataSources[i], i);
}

//#####################################################################################
// Signals

void DataSourcesForm::editDataSource(int index)
{
    if (index < 0 || index >= dataSources.size()) return;

    addDataSourceDialog dlg(this);
    dlg.setDataSource(dataSources[index]);

    if (dlg.exec() == QDialog::Accepted)
    {
        dataSources[index] = dlg.getDataSource();
        refreshDataSourceTable();
    }
}

void DataSourcesForm::removeDataSource(int row)
{
    if (row < 0 || row >= dataSources.size()) return;

    dataSources.removeAt(row);
    refreshDataSourceTable();
}

void DataSourcesForm::on_bttn_create_DS_clicked()
{
    addDataSourceDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        DataSourceData inv = dlg.getDataSource();
        dataSources.append(inv);
        addDataSourceRow(inv, dataSources.size() - 1);
    }
}
