#include "casedetailwizard.h"
#include "ui_casedetailwizard.h"

caseDetailWizard::caseDetailWizard(const CaseManager* caseManager, QWidget *parent) :
    QWizard(parent),
    ui(new Ui::caseDetailWizard),
    currentCaseManager(caseManager)
{
    ui->setupUi(this);

    qDebug() << "Case Details Wizard Opened";

    setOption(QWizard::NoBackButtonOnStartPage);
    setOption(QWizard::NoCancelButton, true);
    setWizardStyle(QWizard::ModernStyle);

    LoadCaseData();

}

caseDetailWizard::~caseDetailWizard()
{
    qDebug() << "Case Details Wizard Destroyed";

    delete ui;
}

//#####################################################################################

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

void caseDetailWizard::on_bttn_create_inv_clicked()
{

}


void caseDetailWizard::on_bttn_create_org_clicked()
{
    //TODO:
}

