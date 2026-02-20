#ifndef ADDORGANIZATIONDIALOG_H
#define ADDORGANIZATIONDIALOG_H

#include <QDialog>
#include "data/case/organizationData.h"

namespace Ui {
class addOrganizationDialog;
}

class addOrganizationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addOrganizationDialog(QWidget *parent = nullptr);
    ~addOrganizationDialog();

    void setOrganization(const OrganizationData &data);
    OrganizationData getOrganization() const;

private:
    Ui::addOrganizationDialog *ui;
};

#endif // ADDORGANIZATIONDIALOG_H
