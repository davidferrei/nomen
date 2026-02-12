#ifndef CASEDETAILWIZARD_H
#define CASEDETAILWIZARD_H

#include <QWizard>

#include "core/caseManager/casemanager.h"

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

private:
    Ui::caseDetailWizard *ui;
    const CaseManager* currentCaseManager;

    void LoadCaseData();
};

#endif // CASEDETAILWIZARD_H
