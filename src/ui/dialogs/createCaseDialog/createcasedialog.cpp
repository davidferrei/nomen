#include "createcasedialog.h"
#include "ui_createcasedialog.h"

#include <QFileDialog>
#include <QMessageBox>

CreateCaseDialog::CreateCaseDialog(QWidget *parent, CaseManager& caseManager)
    : QDialog(parent)
    , ui(new Ui::CreateCaseDialog)
    , caseManager(caseManager)
{
    ui->setupUi(this);

    qDebug() << "Create Case Dialog Opened";

}

CreateCaseDialog::~CreateCaseDialog()
{
    qDebug() << "Create Case Dialog Destroyed";
    delete ui;
}

//############################################################################################
//Signals

void CreateCaseDialog::on_bttn_cancel_clicked()
{
    reject();
}


void CreateCaseDialog::on_bttn_create_clicked()
{
    QString casePath = ui->line_edit_case_path->text().trimmed();
    QString caseName = ui->line_edit_case_name->text().trimmed();
    QString caseId = ui->line_edit_case_id->text().trimmed();
    QString notes = ui->plainTextEdit->toPlainText().trimmed();

    if (casePath.isEmpty() || caseName.isEmpty() || caseId.isEmpty()) {
        QMessageBox::warning(
                this,
                "Missing Information",
                "Case Path, Name and ID are Required."
            );
        return;
    }

    QDir case_dir(casePath);
    if (!case_dir.exists()) {
        QMessageBox::warning(
            this,
            "Invalid Path",
            "The selected Case Directory Does not Exist."
            );
        return;
    }


    if (QFile::exists(case_dir.filePath("case_info.json"))) {
        QMessageBox::warning(
            this,
            "Case Already Exists",
            "A Case Already Exists in the Selected Directory."
            );
        return;
    }

    CaseInfo newCase;
    newCase.caseId      = caseId;
    newCase.caseName    = caseName;
    newCase.createdDate = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    newCase.updatedDate = newCase.createdDate;
    newCase.notes       = notes;
    newCase.caseState   = 1;
    newCase.casePath    = casePath;


    if (!caseManager.createCaseInfo(newCase)) {
        QMessageBox::critical(
            this,
            "Case Creation Failed",
            "Failed to Create the Case"
            );
        return;
    }

    accept();
}


void CreateCaseDialog::on_bttn_browse_fs_clicked()
{
    QString case_dir = QFileDialog::getExistingDirectory(
                            this,
                            "Select Case Directory",
                            QDir::homePath(),
                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                        );

    if (!case_dir.isEmpty()) {
        ui->line_edit_case_path->setText(case_dir);
    }
}

