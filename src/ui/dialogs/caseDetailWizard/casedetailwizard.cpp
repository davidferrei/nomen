#include "casedetailwizard.h"
#include "ui_casedetailwizard.h"

#include <QMessageBox>

caseDetailWizard::caseDetailWizard(const CaseManager* caseManager, QWidget *parent) :
    QWizard(parent),
    ui(new Ui::caseDetailWizard),
    currentCaseManager(caseManager)
{
    ui->setupUi(this);

    //TODO: Hava a way to Assign a Investigator to one or many Organizations

    qDebug() << "Case Details Wizard Opened";

    setOption(QWizard::NoBackButtonOnStartPage);
    setOption(QWizard::NoCancelButton, true);
    setWizardStyle(QWizard::ModernStyle);

    LoadCaseData();

    setupInvestigatorTable();
    setupOrganizationTable();

    connect(ui->checkBox_skip, &QCheckBox::toggled, ui->dataSourceWidget, &QWidget::setDisabled);
    //TODO: Add Validation If enabled dont allow Finish without something in DataSources

    //TODO: Make the Data Source Widget
    //TODO: Save Info to DB

}

caseDetailWizard::~caseDetailWizard()
{
    qDebug() << "Case Details Wizard Destroyed";

    delete ui;
}

//#####################################################################################
// Functions


void caseDetailWizard::LoadCaseData()
{
    qDebug() << "Loading Current Case Data";

    if (!currentCaseManager)
    {
        qWarning() << "Case Manager Pointer Is Null";
        return;
    }

    const auto& currentCase = currentCaseManager->getCurrentCase();

    ui->line_edit_case_name->setText(currentCase.caseName);
    ui->line_edit_case_id->setText(currentCase.caseId);
    ui->line_edit_case_path->setText(currentCase.casePath);
    ui->plainTextEdit->setPlainText(currentCase.notes);
}

void caseDetailWizard::setupInvestigatorTable()
{
    ui->table_inv->setColumnCount(3);
    ui->table_inv->horizontalHeader()->setVisible(false);
    ui->table_inv->verticalHeader()->setVisible(false);

    ui->table_inv->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->table_inv->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->table_inv->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    ui->table_inv->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_inv->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void caseDetailWizard::setupOrganizationTable()
{
    ui->table_org->setColumnCount(3);
    ui->table_org->horizontalHeader()->setVisible(false);
    ui->table_org->verticalHeader()->setVisible(false);

    ui->table_org->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->table_org->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->table_org->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    ui->table_org->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_org->setEditTriggers(QAbstractItemView::NoEditTriggers);

}

void caseDetailWizard::refreshInvestigatorTable()
{
    ui->table_inv->setRowCount(0);
    for (int i = 0; i < investigators.size(); ++i)
        addInvestigatorRow(investigators[i], i);
}

void caseDetailWizard::refreshOrganizationTable()
{
    ui->table_org->setRowCount(0);
    for (int i = 0; i < organizations.size(); ++i)
        addOrganizationRow(organizations[i], i);
}


void caseDetailWizard::addInvestigatorRow(const InvestigatorData& inv, int index)
{
    int row = ui->table_inv->rowCount();
    ui->table_inv->insertRow(row);

    ui->table_inv->setItem(row, 0, new QTableWidgetItem(inv.name));

    QPushButton* editBtn = new QPushButton("Edit");
    ui->table_inv->setCellWidget(row, 1, editBtn);
    connect(editBtn, &QPushButton::clicked, [this, editBtn]() {
        int row = ui->table_inv->indexAt(editBtn->pos()).row();
        editInvestigator(row);
    });

    QPushButton* removeBtn = new QPushButton("Remove");
    ui->table_inv->setCellWidget(row, 2, removeBtn);
    connect(removeBtn, &QPushButton::clicked, [this, removeBtn]() {
        int row = ui->table_inv->indexAt(removeBtn->pos()).row();
        if (row < 0 || row >= investigators.size()) return;
        investigators.removeAt(row);
        refreshInvestigatorTable();
    });

    editBtn->setIcon(QIcon::fromTheme("document-edit", QIcon(":/icons/pencil.png")));
    editBtn->setToolTip("Edit Investigator");
    editBtn->setText("");
    removeBtn->setIcon(QIcon::fromTheme("edit-delete", QIcon(":/icons/trash.png")));
    removeBtn->setToolTip("Remove Investigator");
    removeBtn->setText("");
}

void caseDetailWizard::addOrganizationRow(const OrganizationData& org,int index)
{
    int row = ui->table_org->rowCount();
    ui->table_org->insertRow(row);

    ui->table_org->setItem(row, 0, new QTableWidgetItem(org.name));

    QPushButton* editBtn = new QPushButton();
    editBtn->setIcon(QIcon::fromTheme("document-edit", QIcon(":/icons/pencil.png")));
    editBtn->setToolTip("Edit Organization");
    editBtn->setText("");
    editBtn->setFlat(true);

    ui->table_org->setCellWidget(row, 1, editBtn);

    connect(editBtn, &QPushButton::clicked, [this, editBtn]() {
        int row = ui->table_org->indexAt(editBtn->pos()).row();
        editOrganization(row);
    });

    QPushButton* removeBtn = new QPushButton();
    removeBtn->setIcon(QIcon::fromTheme("edit-delete", QIcon(":/icons/trash.png")));
    removeBtn->setToolTip("Remove Organization");
    removeBtn->setText("");
    removeBtn->setFlat(true);

    ui->table_org->setCellWidget(row, 2, removeBtn);

    connect(removeBtn, &QPushButton::clicked, [this, removeBtn]() {
        int row = ui->table_org->indexAt(removeBtn->pos()).row();
        if (row < 0 || row >= organizations.size()) return;
        organizations.removeAt(row);
        refreshOrganizationTable();
    });
}

//#####################################################################################
// QWizard Logic

bool caseDetailWizard::validateCurrentPage()
{

    switch (currentId())
    {

    // Page 02 - Investigators / Orgs
    case 1:
    {
        qDebug() << "#########################";
        qDebug() << "##### INVESTIGATORS #####";
        for (int i = 0; i < investigators.size(); ++i)
        {
            qDebug() << "Index:" << i
                     << "| Name:" << investigators[i].name
                     << "| Email:" << investigators[i].email
                     << "| Phone:" << investigators[i].phoneNumber;
        }

        qDebug() << "##### ORGANIZATIONS #####";
        for (int i = 0; i < organizations.size(); ++i)
        {
            qDebug() << "Index:" << i
                     << "| Name:" << organizations[i].name
                     << "| Email:" << organizations[i].email
                     << "| Phone:" << organizations[i].phoneNumber;
        }

        qDebug() << "#########################";
        break;
    }

    // Page 03 - Data Source
    case 2:
    {
        if (!ui->checkBox_skip->isChecked())
        {
            QMessageBox::warning(this,
                                 "Data Source Required",
                                 "You must provide a data source or check 'Skip this step'.");
            return false;
        }
        break;
    }

    default:
        break;
    }

    return QWizard::validateCurrentPage();


}


void caseDetailWizard::done(int result)
{
    if (result == QDialog::Accepted) {
        if (!currentCaseManager) {
            QMessageBox::critical(this, "Error", "CaseManager Not Found");
            QDialog::reject();
            return;
        }

        bool allOk = true;

        for (const InvestigatorData& inv : investigators) {
            if (!currentCaseManager->saveInvestigator(inv)) {
                allOk = false;
                qCritical() << "Failed to Save Investigator:" << inv.name;
            }
        }

        if (!allOk) {
            QMessageBox::critical(this, "Error", "One or more investigators could not be saved.");
            return; // stop closing wizard
        }

        QMessageBox::information(this, "Success", "Case details saved!");
    }

    QWizard::done(result);
}

//#####################################################################################
// Signals

void caseDetailWizard::on_bttn_create_inv_clicked()
{
    addInvestigatorDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        InvestigatorData inv = dlg.getInvestigator();
        investigators.append(inv);
        addInvestigatorRow(inv, investigators.size() - 1);
    }

}

void caseDetailWizard::editInvestigator(int index)
{
    if (index < 0 || index >= investigators.size()) return;

    addInvestigatorDialog dlg(this);
    dlg.setInvestigator(investigators[index]);

    if (dlg.exec() == QDialog::Accepted)
    {
        investigators[index] = dlg.getInvestigator();
        refreshInvestigatorTable();
    }
}


void caseDetailWizard::removeInvestigator(int row)
{
    if (row < 0 || row >= ui->table_inv->rowCount()) return;
    ui->table_inv->removeRow(row);
}

void caseDetailWizard::on_bttn_create_org_clicked()
{
    addOrganizationDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        OrganizationData org = dlg.getOrganization();
        organizations.append(org);
        addOrganizationRow(org, investigators.size() - 1);

    }
}


void caseDetailWizard::editOrganization(int index)
{
    if (index < 0 || index >= organizations.size()) return;

    addOrganizationDialog dlg(this);
    dlg.setOrganization(organizations[index]);

    if (dlg.exec() == QDialog::Accepted)
    {
        organizations[index] = dlg.getOrganization();
        refreshOrganizationTable();
    }
}


void caseDetailWizard::removeOrganization(int row)
{
    if (row < 0 || row >= ui->table_org->rowCount()) return;
    ui->table_org->removeRow(row);
}




