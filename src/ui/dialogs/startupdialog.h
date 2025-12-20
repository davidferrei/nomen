#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <QDialog>

namespace Ui {
class startupDialog;
}

class startupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit startupDialog(QWidget *parent = nullptr);
    ~startupDialog();

private slots:
    void on_pushButton_clicked();

    void on_table_case_cellDoubleClicked(int row, int column);

private:
    Ui::startupDialog *ui;

    void populateCaseTable();

};

#endif // STARTUPDIALOG_H
