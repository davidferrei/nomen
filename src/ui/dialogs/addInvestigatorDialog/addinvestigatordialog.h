#ifndef ADDINVESTIGATORDIALOG_H
#define ADDINVESTIGATORDIALOG_H

#include <QDialog>

#include "data/case/investigatorData.h"

namespace Ui {
class addInvestigatorDialog;
}

class addInvestigatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addInvestigatorDialog(QWidget *parent = nullptr);
    ~addInvestigatorDialog();

    void setInvestigator(const InvestigatorData& data);

    InvestigatorData getInvestigator() const;

private:
    Ui::addInvestigatorDialog *ui;
};

#endif // ADDINVESTIGATORDIALOG_H
