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

private:
    Ui::startupDialog *ui;
};

#endif // STARTUPDIALOG_H
