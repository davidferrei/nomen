#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include "core/caseManager/casemanager.h"

#include <QDialog>

class CaseManager;
class CreateCaseDialog;

namespace Ui {
class startupDialog;
}

class startupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit startupDialog(QWidget *parent, CaseManager& caseManager);
    ~startupDialog();

private slots:
    void on_pushButton_clicked();

    void on_table_case_cellDoubleClicked(int row, int column);

    void on_bttn_create_case_clicked();

private:
    Ui::startupDialog *ui;

    CaseManager& caseManager;

    void populateCaseTable();

};

#endif // STARTUPDIALOG_H
