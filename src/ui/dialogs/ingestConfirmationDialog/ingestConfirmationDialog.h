#ifndef INGESTCONFIRMATIONDIALOG_H
#define INGESTCONFIRMATIONDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class ingestConfirmationDialog;
}

class ingestConfirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ingestConfirmationDialog(const QStringList &modulesList = QStringList(), QWidget *parent = nullptr);
    ~ingestConfirmationDialog();

private:
    Ui::ingestConfirmationDialog *ui;
};

#endif // INGESTCONFIRMATIONDIALOG_H
