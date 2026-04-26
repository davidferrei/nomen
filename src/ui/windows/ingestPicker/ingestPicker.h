#ifndef INGESTPICKER_H
#define INGESTPICKER_H

#include "core/extensions/ingestModuleFactory.h"

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QJsonDocument>
#include <QItemSelection>
#include <QItemSelectionModel>

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
    IngestModuleFactory *m_factory = nullptr;
};

#endif // INGESTPICKER_H
