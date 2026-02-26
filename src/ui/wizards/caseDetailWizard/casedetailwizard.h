#ifndef CASEDETAILWIZARD_H
#define CASEDETAILWIZARD_H

#include <QWizard>

#include "core/caseManager/casemanager.h"
#include "data/case/investigatorData.h"
#include "data/case/organizationData.h"


#include "ui/dialogs/addInvestigatorDialog/addinvestigatordialog.h"
#include "ui/dialogs/addOrganizationDialog/addorganizationdialog.h"
#include "../../widgets/dataSourcesForm.h"



namespace Ui {
class caseDetailWizard;
}

class caseDetailWizard : public QWizard
{
    Q_OBJECT

public:
    explicit caseDetailWizard(CaseManager* caseManager, QWidget *parent = nullptr);
    ~caseDetailWizard();

private slots:
    void on_bttn_create_inv_clicked();

    void on_bttn_create_org_clicked();

    bool validateCurrentPage() override;

protected:
    void done(int result) override;

private:
    Ui::caseDetailWizard *ui;
    CaseManager* currentCaseManager;

    QList<InvestigatorData> investigators;
    QList<OrganizationData> organizations;
    DataSourcesForm* dataSourceWidget;

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
