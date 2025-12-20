#include "startupdialog.h"
#include "ui_startupdialog.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

startupDialog::startupDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::startupDialog)
{
    ui->setupUi(this);

    qDebug() << "Startup Dialog Opened";

    //Widgets Setup
    // ui->table_case->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
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
    const QString caseJsonPath = "/home/admin1/NomenCases/TestCase01/case_info.json";
    const QString caseRootPath = "/home/admin1/NomenCases/TestCase01";

    QFile file(caseJsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to Open Case File in " << caseJsonPath;
        return;
    }

    QJsonDocument jdoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!jdoc.isObject()) {
        qWarning() << "Invalid JSON Format";
        return;
    }

    QJsonObject caseInfoObj = jdoc.object();

    QString caseName = caseInfoObj.value("case_name").toString();
    QString caseID = caseInfoObj.value("case_id").toString();
    QString createdDate = caseInfoObj.value("created_date").toString();
    QString updatedDate = caseInfoObj.value("updated_date").toString();
    QString caseNotes = caseInfoObj.value("notes").toString();
    int caseState = caseInfoObj.value("case_state").toInt();

    ui->table_case->setRowCount(10); //TODO: Make this Dinamic

    for (int i = 0; i < 10; ++i) {
        ui->table_case->setItem(i, 0, new QTableWidgetItem(caseName));
        ui->table_case->setItem(i, 1, new QTableWidgetItem(caseID));

        QTableWidgetItem *stateItem = new QTableWidgetItem();
        if (caseState == 1) {
            stateItem->setText("Active");
            stateItem->setForeground(QBrush(QColor(0, 120, 0)));
        } else {
            stateItem->setText("Closed");
            stateItem->setForeground(QBrush(QColor(120, 0, 0)));
        }
        ui->table_case->setItem(i, 2, stateItem);

        ui->table_case->setItem(i, 3, new QTableWidgetItem(createdDate));
        ui->table_case->setItem(i, 4, new QTableWidgetItem(updatedDate));
        ui->table_case->setItem(i, 5, new QTableWidgetItem(caseNotes));

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
    accept();
}
