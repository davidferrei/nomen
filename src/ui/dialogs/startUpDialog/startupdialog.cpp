#include "startupdialog.h"
#include "ui_startupdialog.h"

#include "core/caseManager/casemanager.h"
#include "ui/dialogs/createCaseDialog/createcasedialog.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

//TODO: Implement Creating
//TODO: Implement Searching
//TODO: Implement Sorting

startupDialog::startupDialog(QWidget *parent, CaseManager& caseManager)
    : QDialog(parent)
    , ui(new Ui::startupDialog)
    , caseManager(caseManager)
{
    ui->setupUi(this);

    qDebug() << "Startup Dialog Opened";

    //Widgets Setup
    ui->table_case->horizontalHeader()->setStretchLastSection(true);

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

    QList<CaseInfo> casesList = caseManager.loadPreviousCases();

    ui->table_case->clearContents();
    ui->table_case->setRowCount(casesList.size());

    for (int i = 0; i < casesList.size(); ++i) {
        const CaseInfo& c = casesList[i];

        ui->table_case->setItem(i, 0, new QTableWidgetItem(c.caseName));
        ui->table_case->setItem(i, 1, new QTableWidgetItem(c.caseId));

        QTableWidgetItem* stateItem = new QTableWidgetItem;
        if (c.caseState == 1) {
            stateItem->setText("Active");
            stateItem->setForeground(QBrush(QColor(0, 120, 0)));
        } else {
            stateItem->setText("Closed");
            stateItem->setForeground(QBrush(QColor(120, 0, 0)));
        }
        ui->table_case->setItem(i, 2, stateItem);

        ui->table_case->setItem(i, 3, new QTableWidgetItem(c.createdDate));
        ui->table_case->setItem(i, 4, new QTableWidgetItem(c.updatedDate));
        ui->table_case->setItem(i, 5, new QTableWidgetItem(c.notes));
    }

    ui->table_case->resizeColumnToContents(2);
}

//############################################################################################
//Signals

void startupDialog::on_table_case_cellDoubleClicked(int row, int column)
{
    //TODO: Start Case
}

void startupDialog::on_pushButton_clicked()
{
    //TODO: Start Case
    accept();
}

void startupDialog::on_bttn_create_case_clicked()
{
    CreateCaseDialog createDlg(this);

    if (createDlg.exec() != QDialog::Accepted)
        return;

    // CaseInfo info = createDlg.caseInfo();

    // if (!caseManager.createCase(info)) {
    //     // QMessageBox::warning(this, "Error", "Failed to create case");
    //     return;
    // }

    accept();
}

