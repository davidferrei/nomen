#include "startupdialog.h"
#include "ui_startupdialog.h"

#include "core/caseManager/casemanager.h"
#include "ui/dialogs/createCaseDialog/createcasedialog.h"
#include "core/logging/performancemeasure.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDebug>


//TODO: Implement Searching
//TODO: Implement Sorting

//TODO: Add a Info Button for Each Row that Reads from the DB and Displays Case Info. (Examiners,Organizations,Stats)

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
    PerformanceTimer timer("Populate Previous Cases Table");

    QList<CaseInfo> casesList = caseManager.loadPreviousCases();

    ui->table_case->clearContents();
    ui->table_case->setRowCount(casesList.size());

    for (int i = 0; i < casesList.size(); ++i) {
        const CaseInfo& c = casesList[i];

        QTableWidgetItem* nameItem = new QTableWidgetItem(c.caseName);
        nameItem->setData(Qt::UserRole, c.casePath);
        ui->table_case->setItem(i, 0, nameItem);
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

        ui->table_case->setItem(i, 3, new QTableWidgetItem(QDateTime::fromString(c.createdDate, Qt::ISODate).toLocalTime().toString("dd/MM/yyyy HH:mm:ss")));
        ui->table_case->setItem(i, 4, new QTableWidgetItem(QDateTime::fromString(c.updatedDate, Qt::ISODate).toLocalTime().toString("dd/MM/yyyy HH:mm:ss")));

        ui->table_case->setItem(i, 5, new QTableWidgetItem(c.notes.simplified()));
    }

    ui->table_case->resizeColumnToContents(2);
    ui->table_case->resizeColumnToContents(3);
    ui->table_case->resizeColumnToContents(4);
}

//############################################################################################
//Signals

void startupDialog::on_table_case_cellDoubleClicked(int row, int column)
{
    QString casePath = ui->table_case->item(row, 0)->data(Qt::UserRole).toString();

    if (!caseManager.openCase(casePath)) {
        QMessageBox::critical(this,"Error","The Selected Case Could not be Opened.");
        return;
    }

    accept();
}

void startupDialog::on_pushButton_clicked()
{
    int row = ui->table_case->currentRow();
    if (row < 0)
        return; //TODO: Disable Button Until Something is Selected

    QString casePath = ui->table_case->item(row, 0)->data(Qt::UserRole).toString();

    if (!caseManager.openCase(casePath)) {
        QMessageBox::critical(this, "Error", "The Selected Case Could not be Opened.");
        return;
    }

    accept();
}

void startupDialog::on_bttn_create_case_clicked()
{
    CreateCaseDialog createDlg(this, caseManager);

    if (createDlg.exec() != QDialog::Accepted)
        return;

    populateCaseTable();

    //TODO: Open Case
    //accept();
}

