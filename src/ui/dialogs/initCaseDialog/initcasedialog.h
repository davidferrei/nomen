#ifndef INITCASEDIALOG_H
#define INITCASEDIALOG_H

#include <QDialog>

namespace Ui {
class initCaseDialog;
}

class initCaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit initCaseDialog(QWidget *parent = nullptr);
    ~initCaseDialog();

private:
    Ui::initCaseDialog *ui;
};

#endif // INITCASEDIALOG_H
