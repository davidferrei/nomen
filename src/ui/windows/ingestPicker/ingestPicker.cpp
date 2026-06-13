#include "ingestPicker.h"
#include "ui_ingestPicker.h"

ingestPicker::ingestPicker(IngestModuleFactory &moduleFactory, const QList<DataSourceData> &dsList, JobManager* jobMgr, const QString &casePath, QWidget *parent) : QMainWindow(parent), ui(new Ui::ingestPicker), jobManager(jobMgr) , dataSourcesList(dsList), caseFolder(casePath)
{
    ui->setupUi(this);

    ui->DSComboBox->clear();
    ui->DSComboBox->addItem(QStringLiteral("Select a Disk..."), -1);
    for (int i = 0; i < dataSourcesList.size(); ++i)
        ui->DSComboBox->addItem(dataSourcesList.at(i).name, i);

    ui->startIngestBttn->setEnabled(false);

    setupModulesTable();
    populateModulesTable(moduleFactory.moduleNamesList());

    //Update Metadata Tab (Main)
    connect(ui->ingestModulesTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        const int row = ui->ingestModulesTable->currentRow();
        if (row < 0) return;
        QTableWidgetItem *nameItem = ui->ingestModulesTable->item(row, 1);
        if (!nameItem) return;
        const QString moduleName = nameItem->text();
        const QJsonObject moduleMetadata = IngestModuleFactory::instance().getModuleMetadata(moduleName);

        ui->NameValue->setText(moduleMetadata.value("name").toString());
        ui->ModuleNameLabel->setText(moduleMetadata.value("name").toString());
        ui->VersionValue->setText(moduleMetadata.value("version").toString());
        ui->AuthorValue->setText(moduleMetadata.value("author").toString());
        ui->ModuleIDValue->setText(moduleMetadata.value("id").toString());
        ui->moduleDescriptionEdit->setPlainText(moduleMetadata.value("description").toString());
    });

    //Update Selected Modules List
    connect(ui->ingestModulesTable, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item) {
        if (item->column() != 0) return;

        const int row = item->row();
        QTableWidgetItem *nameItem = ui->ingestModulesTable->item(row, 1);
        if (!nameItem)
        {
            qWarning() << "[IngestPicker] Check State Changed but Name Item is NULL, Row" << row;
            return;
        }

        const QString moduleName = nameItem->text();

        if (item->checkState() == Qt::Checked)
        {
            if (!selectedModules.contains(moduleName))
                selectedModules.append(moduleName);
        }
        else
        {
            selectedModules.removeAll(moduleName);
        }

        const bool dsChosen = (ui->DSComboBox->currentData().toInt() != -1);
        ui->startIngestBttn->setEnabled(!selectedModules.isEmpty() && dsChosen);
    });

    connect(ui->DSComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int){
        const bool dsChosen = (ui->DSComboBox->currentData().toInt() != -1);
        ui->startIngestBttn->setEnabled(!selectedModules.isEmpty() && dsChosen);
    });

    connect(ui->startIngestBttn, &QPushButton::clicked, this, [this]() {
        ingestConfirmationDialog confirmationDialog(selectedModules, this);
        if (confirmationDialog.exec() == QDialog::Accepted)
        {
            const int dsIndex = ui->DSComboBox->currentData().toInt();
            if (dsIndex < 0 || dsIndex >= dataSourcesList.size()) 
            {
                qWarning() << "[IngestPicker] Invalid DS Index:" << dsIndex;
                return;
            }

            const DataSourceData &ds = dataSourcesList.at(dsIndex);
            qDebug() << "[IngestPicker] Starting Ingest for DataSource:" << ds.name << "with Modules:" << selectedModules;
            for (const QString &moduleName : selectedModules)
            {
                jobManager->CreateIngestJob(ds, caseFolder, moduleName);
            }

            close();
        }
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
    ui->ingestModulesTable->setColumnWidth(0, 5);

    ui->ingestModulesTable->horizontalHeader()->setSectionsMovable(false);
}

void ingestPicker::populateModulesTable(const QStringList &modulesList)
{
    ui->ingestModulesTable->setRowCount(modulesList.size());

    ui->ingestModulesTable->blockSignals(true);
    selectedModules.clear();

    for (int r = 0; r < modulesList.size(); ++r)
    {
        QTableWidgetItem *checkColumn = new QTableWidgetItem(QString());
        checkColumn->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        checkColumn->setCheckState(Qt::Unchecked);
        checkColumn->setTextAlignment(Qt::AlignCenter);
        ui->ingestModulesTable->setItem(r, 0, checkColumn);

        QTableWidgetItem *name = new QTableWidgetItem(modulesList.at(r));
        ui->ingestModulesTable->setItem(r, 1, name);
    }

    ui->ingestModulesTable->blockSignals(false);
}
