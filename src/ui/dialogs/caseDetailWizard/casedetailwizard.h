#ifndef CASEDETAILWIZARD_H
#define CASEDETAILWIZARD_H

#include <QWizard>

#include "core/caseManager/casemanager.h"
#include "data/case/investigatorData.h"
#include "data/case/organizationData.h"


#include "ui/dialogs/addInvestigatorDialog/addinvestigatordialog.h"
#include "ui/dialogs/addOrganizationDialog/addorganizationdialog.h"


namespace Ui {
class caseDetailWizard;
}

class caseDetailWizard : public QWizard
{
    Q_OBJECT

public:
    explicit caseDetailWizard(const CaseManager* caseManager, QWidget *parent = nullptr);
    ~caseDetailWizard();

private slots:
    void on_bttn_create_inv_clicked();

    void on_bttn_create_org_clicked();

    bool validateCurrentPage() override;

private:
    Ui::caseDetailWizard *ui;
    const CaseManager* currentCaseManager;

    QList<InvestigatorData> investigators;
    QList<OrganizationData> organizations;

    void LoadCaseData();
    void setupInvestigatorTable();
    void setupOrganizationTable();
    void refreshInvestigatorTable();
    void refreshOrganizationTable();


    void editInvestigator(int row);
    void removeInvestigator(int row);

    void editOrganization(int row);
    void removeOrganization(int row);


    void addInvestigatorRow(const InvestigatorData &inv, int index);
    void addOrganizationRow(const OrganizationData& org, int index);

};

#endif // CASEDETAILWIZARD_H
