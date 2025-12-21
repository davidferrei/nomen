#ifndef CREATECASEDIALOG_H
#define CREATECASEDIALOG_H

#include "core/caseManager/casemanager.h"


#include <QDialog>

namespace Ui {
class CreateCaseDialog;
}

class CreateCaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateCaseDialog(QWidget *parent, CaseManager& caseManager);
    ~CreateCaseDialog();

private slots:
    void on_bttn_cancel_clicked();

    void on_bttn_create_clicked();

    void on_bttn_browse_fs_clicked();

private:
    Ui::CreateCaseDialog *ui;

    CaseManager& caseManager;
};

#endif // CREATECASEDIALOG_H
