#ifndef ADDINVESTIGATORDIALOG_H
#define ADDINVESTIGATORDIALOG_H

#include <QDialog>

namespace Ui {
class addInvestigatorDialog;
}


class addInvestigatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addInvestigatorDialog(QWidget *parent = nullptr);
    ~addInvestigatorDialog();

private:
    Ui::addInvestigatorDialog *ui;
};

#endif // ADDINVESTIGATORDIALOG_H
