#include "adddatasourcedialog.h"
#include "ui_adddatasourcedialog.h"

//TODO: Make TimeZone be a ComboBox

addDataSourceDialog::addDataSourceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addDataSourceDialog)
{
    ui->setupUi(this);

    qDebug() << "Add Data Source Dialog Opened";

}

addDataSourceDialog::~addDataSourceDialog()
{
    qDebug() << "Add Data Source Dialog Destroyed";
    delete ui;
}


void addDataSourceDialog::setDataSource(const DataSourceData &data)
{
    // TODO: Add Validations
    ui->le_name->setText(data.name);
    ui->le_hash->setText(data.hash);
    ui->le_filePath->setText(data.filePath);
    ui->le_timeZone->setText(data.timeZone);
    ui->te_notes->setPlainText(data.notes);
}

DataSourceData addDataSourceDialog::getDataSource() const
{
    DataSourceData dataSource;

    dataSource.name = ui->le_name->text();
    dataSource.hash = ui->le_hash->text();
    dataSource.filePath = ui->le_filePath->text();
    dataSource.timeZone = ui->le_timeZone->text();
    dataSource.notes = ui->te_notes->toPlainText();

    return dataSource;
}

void addDataSourceDialog::on_bttn_browse_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Forensic Image",
        QDir::homePath(),
        "All Files (*)"
        );

    if (filePath.isEmpty())
        return;

    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists())
    {
        QMessageBox::warning(
            this,
            "Invalid File",
            "The Selected File Does not Exist."
            );
        return;
    }

    ui->le_filePath->setText(filePath);
}

