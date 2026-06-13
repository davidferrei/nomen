#ifndef INGESTPICKER_H
#define INGESTPICKER_H

#include "core/extensions/ingestModuleFactory.h"
#include "core/jobManager/jobManager.h"
#include "data/evidence/dataSourceData.h"

#include "../../dialogs/ingestConfirmationDialog/ingestConfirmationDialog.h"

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QJsonDocument>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QStringList>
#include <QVariant>


namespace Ui {
class ingestPicker;
}

class ingestPicker : public QMainWindow
{
    Q_OBJECT

public:
    explicit ingestPicker(IngestModuleFactory &moduleFactory, const QList<DataSourceData> &dsList, JobManager* jobManager, const QString &casePath, QWidget *parent = nullptr);
    ~ingestPicker();

private:
    Ui::ingestPicker *ui;
    void setupModulesTable();
    void populateModulesTable(const QStringList &modulesList);
    QStringList selectedModules;

    JobManager* jobManager = nullptr;
    QList<DataSourceData> dataSourcesList;
    QString caseFolder;
};

#endif // INGESTPICKER_H
