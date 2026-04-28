#ifndef INGESTPICKER_H
#define INGESTPICKER_H

#include "core/extensions/ingestModuleFactory.h"

#include "../../dialogs/ingestConfirmationDialog/ingestConfirmationDialog.h"


#include <QMainWindow>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QJsonDocument>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QStringList>

namespace Ui {
class ingestPicker;
}

class ingestPicker : public QMainWindow
{
    Q_OBJECT

public:
    explicit ingestPicker(IngestModuleFactory &moduleFactory, QWidget *parent = nullptr);
    ~ingestPicker();

private:
    Ui::ingestPicker *ui;
    void setupModulesTable();
    void populateModulesTable(const QStringList &modulesList);
    QStringList selectedModules;
};

#endif // INGESTPICKER_H
