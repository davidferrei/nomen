#ifndef ADDDATASOURCEDIALOG_H
#define ADDDATASOURCEDIALOG_H

#include <QDialog>

#include "data/evidence/dataSourceData.h"


namespace Ui {
class addDataSourceDialog;
}

class addDataSourceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addDataSourceDialog(QWidget *parent = nullptr);
    ~addDataSourceDialog();

    void setDataSource(const DataSourceData &data);
    DataSourceData getDataSource() const;

private:
    Ui::addDataSourceDialog *ui;
};

#endif // ADDDATASOURCEDIALOG_H
