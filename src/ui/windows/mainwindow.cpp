#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "../widgets/DraggableTabWidget.h"


MainWindow::MainWindow(QWidget *parent, CaseManager& caseManager) : QMainWindow(parent), ui(new Ui::MainWindow), caseManager(caseManager)
{
    ui->setupUi(this);

    qDebug() << "Main Window Opened";

    setupUiElements();

    //################################
    // Indexing

    indexManager = new IndexManager(this);

    loadDataSources();

}

MainWindow::~MainWindow()
{
    delete ui;
}

//############################################################################################
//UI Stuff

void MainWindow::setUIWidgetsSizes() 
{
    // Splitters
    QSplitter *v = ui->verticalSplitter;
    if (v) 
    {
        int total = v->height();
        if (total > 0) 
        {
            v->setSizes({static_cast<int>(total * 0.75), static_cast<int>(total * 0.25)});
        } 
        else 
        {
            v->setSizes({600, 200});
        }
    }

    QSplitter *h = ui->horizontalSplitter;
    if (h) 
    {
        int w = h->width();
        if (w > 0) 
        {
            h->setSizes({static_cast<int>(w * 0.25), static_cast<int>(w * 0.75)});
        } 
        else 
        {
            h->setSizes({200, 600});
        }
    }

    // Tab Groups
    if (ui->MainTabGroup)
    {
        DraggableTabWidget *newMain = new DraggableTabWidget(this);
        while (ui->MainTabGroup->count() > 0) 
        {
            QWidget *w = ui->MainTabGroup->widget(0);
            QString t = ui->MainTabGroup->tabText(0);
            QIcon ic = ui->MainTabGroup->tabIcon(0);
            ui->MainTabGroup->removeTab(0);
            newMain->addTab(w, ic, t);
        }
        QSplitter *h = ui->horizontalSplitter;
        if (h) 
        {
            int idx = h->indexOf(ui->MainTabGroup);
            if (idx >= 0) h->replaceWidget(idx, newMain);
            else h->addWidget(newMain);
        }
    }

    if (ui->BottomTabGroup)
    {
        DraggableTabWidget *newBottom = new DraggableTabWidget(this);
        while (ui->BottomTabGroup->count() > 0) 
        {
            QWidget *w = ui->BottomTabGroup->widget(0);
            QString t = ui->BottomTabGroup->tabText(0);
            QIcon ic = ui->BottomTabGroup->tabIcon(0);
            ui->BottomTabGroup->removeTab(0);
            newBottom->addTab(w, ic, t);
        }
        QSplitter *v = ui->verticalSplitter;
        if (v) 
        {
            int idx = v->indexOf(ui->BottomTabGroup);
            if (idx >= 0) v->replaceWidget(idx, newBottom);
            else v->addWidget(newBottom);
        }
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    setUIWidgetsSizes();
}

void MainWindow::setupUiElements()
{
    // Menu Bar
    QMenu* fileMenu = ui->menubar->addMenu("&Nomen");
    QAction* openAction = new QAction("Open Case", this);
    fileMenu->addAction(openAction);
    QAction* exitAction = new QAction("Exit", this);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(exitAction);

    openAction->setEnabled(false);
    exitAction->setEnabled(true);

    QMenu* addMenu = ui->menubar->addMenu("&Add");
    QAction* newDSAction = new QAction("New Data Sources", this);
    addMenu->addAction(newDSAction);
    QAction* newIngestJobAction = new QAction("New Ingest Job", this);
    addMenu->addAction(newIngestJobAction);

    newDSAction->setEnabled(false);
    newIngestJobAction->setEnabled(false);

    QMenu* addView = ui->menubar->addMenu("&View");
    QAction* viewCaseInfoAction = new QAction("Case Information", this);
    addView->addAction(viewCaseInfoAction);
    QAction* viewDSAction = new QAction("Data Sources", this);
    addView->addAction(viewDSAction);
    QAction* viewJobsAction = new QAction("Jobs", this);
    addView->addAction(viewJobsAction);

    viewCaseInfoAction->setEnabled(false);
    viewDSAction->setEnabled(false);
    viewJobsAction->setEnabled(false);

    QMenu* addSettings = ui->menubar->addMenu("&Settings");
    QAction* settingsNomenAction = new QAction("Nomen", this);
    addSettings->addAction(settingsNomenAction);

    settingsNomenAction->setEnabled(false);



    QMenu* helpMenu = ui->menubar->addMenu("&Help");
    QAction* aboutAction = new QAction("About", this);
    aboutAction->setIcon(QIcon(":/icons/app_icon_v1.svg"));
    connect(aboutAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "About", "Nomen v1.0");
    });
    helpMenu->addAction(aboutAction);

    // Status Bar
    ui->statusbar->showMessage("Ready");

    const CaseInfo& c = caseManager.getCurrentCase();
    QLabel* userLabel = new QLabel(
        "Case: " + c.caseName + " | " + c.caseId,
        this
        );
    ui->statusbar->addPermanentWidget(userLabel);
}

//############################################################################################
// Logic

void MainWindow::loadDataSources()
{
    QList<DataSourceData> dataSources = caseManager.getDataSources();

    qDebug() << "Total Disks Found:" << dataSources.size();

    caseTreeModel = new CaseTreeModel(this);

    for (const DataSourceData& ds : dataSources)
    {
        qDebug().noquote() << "========== DataSource ==========";
        qDebug().noquote() << "ID          :" << ds.id;
        qDebug().noquote() << "Name        :" << ds.name;
        qDebug().noquote() << "File Path   :" << ds.filePath;
        qDebug().noquote() << "Indexed     :" << (ds.indexed ? "YES" : "NO");
        qDebug().noquote() << "================================";

        if (ds.indexed)
        {
            QString lmdbPath = caseManager.getCurrentCase().casePath + "/indexes/disk-" + QString::number(ds.id);

            caseTreeModel->addDataSource(
                                        static_cast<int>(ds.id), 
                                        ds.name.isEmpty() ? "Disk " + QString::number(ds.id) : ds.name, 
                                        lmdbPath
            );
        }
        else
        {
            qDebug() << "Disk Requires Ingest:" << ds.filePath;
            indexManager->CreateJob(ds, caseManager.getCurrentCase().casePath);
            //TODO: Add to CaseTreeModel
        }
    }

    ui->treeViewFilesystem->setModel(caseTreeModel);
    ui->treeViewFilesystem->setHeaderHidden(true);

    ui->treeViewFilesystem->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->treeViewFilesystem->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QHeaderView *hv = ui->treeViewFilesystem->header();
    if (hv) 
    {
        hv->setSectionResizeMode(0, QHeaderView::Interactive);
        hv->setStretchLastSection(false);
    }

    ui->treeViewFilesystem->resizeColumnToContents(0);
    connect(caseTreeModel, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex&, int, int){
        ui->treeViewFilesystem->resizeColumnToContents(0);
    });
}


