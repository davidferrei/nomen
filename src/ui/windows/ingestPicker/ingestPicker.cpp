#include "ingestPicker.h"
#include "ui_ingestPicker.h"


ingestPicker::ingestPicker(IngestModuleFactory &moduleFactory, QWidget *parent) : QMainWindow(parent), ui(new Ui::ingestPicker)
{
    ui->setupUi(this);

    setupModulesTable();
    populateModulesTable(moduleFactory.moduleNamesList());

    connect(ui->ingestModulesTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        const int row = ui->ingestModulesTable->currentRow();
        if (row < 0) return;
        QTableWidgetItem *nameItem = ui->ingestModulesTable->item(row, 1);
        if (!nameItem) return;
        const QString moduleName = nameItem->text();
        const QJsonObject moduleMetadata = IngestModuleFactory::instance().getModuleMetadata(moduleName);
        qDebug() << "[ingestPicker] Module Metadata:" << QJsonDocument(moduleMetadata).toJson(QJsonDocument::Compact);
    });
}

ingestPicker::~ingestPicker()
{
    delete ui;
}

//############################################################################################
//UI Stuff

void ingestPicker::setupModulesTable()
{
    ui->ingestModulesTable->clear();
    ui->ingestModulesTable->setColumnCount(2);
    ui->ingestModulesTable->setHorizontalHeaderLabels({QStringLiteral(""), QStringLiteral("Module Name")});

    ui->ingestModulesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ingestModulesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->ingestModulesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ingestModulesTable->setDragEnabled(false);
    ui->ingestModulesTable->setDragDropMode(QAbstractItemView::NoDragDrop);

    ui->ingestModulesTable->verticalHeader()->setVisible(false);

    ui->ingestModulesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->ingestModulesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->ingestModulesTable->setColumnWidth(0, 50);

    ui->ingestModulesTable->horizontalHeader()->setSectionsMovable(false);
}

void ingestPicker::populateModulesTable(const QStringList &modulesList)
{
    ui->ingestModulesTable->setRowCount(modulesList.size());

    for (int r = 0; r < modulesList.size(); ++r)
    {
        QTableWidgetItem *checkColumn = new QTableWidgetItem(QString());
        ui->ingestModulesTable->setItem(r, 0, checkColumn);

        QTableWidgetItem *name = new QTableWidgetItem(modulesList.at(r));
        ui->ingestModulesTable->setItem(r, 1, name);
    }
}
