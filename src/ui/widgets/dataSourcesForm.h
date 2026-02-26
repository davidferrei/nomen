#ifndef DATASOURCESFORM_H
#define DATASOURCESFORM_H

#include <QWidget>

#include "data/evidence/dataSourceData.h"


#include "ui/dialogs/addDataSourceDialog/adddatasourcedialog.h"


namespace Ui {
class DataSourcesForm;
}

class DataSourcesForm : public QWidget
{
    Q_OBJECT

public:
    explicit DataSourcesForm(QWidget *parent = nullptr);
    ~DataSourcesForm();

    QList<DataSourceData> getDataSources() const;

private slots:
    void on_bttn_create_DS_clicked();

private:
    Ui::DataSourcesForm *ui;

    QList<DataSourceData> dataSources;

    void setupDataSourceTable();
    void refreshDataSourceTable();
    void addDataSourceRow(const DataSourceData& ds, int index);

    void removeDataSource(int row);
    void editDataSource(int index);
};

#endif // DATASOURCESFORM_H
