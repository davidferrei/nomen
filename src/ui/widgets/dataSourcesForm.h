#ifndef DATASOURCESFORM_H
#define DATASOURCESFORM_H

#include <QWidget>

namespace Ui {
class DataSourcesForm;
}

class DataSourcesForm : public QWidget
{
    Q_OBJECT

public:
    explicit DataSourcesForm(QWidget *parent = nullptr);
    ~DataSourcesForm();

private:
    Ui::DataSourcesForm *ui;
};

#endif // DATASOURCESFORM_H
