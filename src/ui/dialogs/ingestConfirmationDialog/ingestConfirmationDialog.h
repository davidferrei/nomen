#ifndef INGESTCONFIRMATIONDIALOG_H
#define INGESTCONFIRMATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ingestConfirmationDialog;
}

class ingestConfirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ingestConfirmationDialog(QWidget *parent = nullptr);
    ~ingestConfirmationDialog();

private:
    Ui::ingestConfirmationDialog *ui;
};

#endif // INGESTCONFIRMATIONDIALOG_H
