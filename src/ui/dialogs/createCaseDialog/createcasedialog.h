#ifndef CREATECASEDIALOG_H
#define CREATECASEDIALOG_H

#include <QDialog>

namespace Ui {
class CreateCaseDialog;
}

class CreateCaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateCaseDialog(QWidget *parent = nullptr);
    ~CreateCaseDialog();

private:
    Ui::CreateCaseDialog *ui;
};

#endif // CREATECASEDIALOG_H
