#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "../../core/index/repo/IndexDBManager.h"


MainWindow::MainWindow(QWidget *parent, CaseManager& caseManager)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , caseManager(caseManager)
{
    ui->setupUi(this);

    qDebug() << "Main Window Opened";

    //qInfo() << "Start Disk Walk";
    //TskWalker::walkImage("/mnt/disk01/DiskImage/kali.raw");

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

void MainWindow::setUIWidgetsSizes() {

    int totalHeight = ui->centralwidget->height();
    int bottomHeight = totalHeight * 0.25;
    int topHeight = totalHeight - bottomHeight;

    ui->verticalLayout->setStretch(0, topHeight);
    ui->verticalLayout->setStretch(1, bottomHeight);

    int totalWidth = ui->MainLayoutGroup->geometry().width();
    int leftWidth = totalWidth * 0.25;
    int rightWidth = totalWidth - leftWidth;

    ui->MainLayoutGroup->setStretch(0, leftWidth);
    ui->MainLayoutGroup->setStretch(1, rightWidth);
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
    //TODO: Save DataSources Window wide?
    QList<DataSourceData> dataSources = caseManager.getDataSources();

    qDebug() << "Total Disks Found:" << dataSources.size();

    for (const DataSourceData& ds : dataSources)
    {
        qDebug().noquote() << "========== DataSource ==========";
        qDebug().noquote() << "ID          :" << ds.id;
        qDebug().noquote() << "Name        :" << ds.name;
        qDebug().noquote() << "Disk Size   :" << ds.diskSize << "bytes";
        qDebug().noquote() << "Sector Size :" << ds.sectorSize;
        qDebug().noquote() << "Hash        :" << ds.hash;
        qDebug().noquote() << "Hash Type   :" << ds.hashType;
        qDebug().noquote() << "Format      :" << ds.format;
        qDebug().noquote() << "File Path   :" << ds.filePath;
        qDebug().noquote() << "Time Zone   :" << ds.timeZone;
        qDebug().noquote() << "Indexed     :" << (ds.indexed ? "YES" : "NO");
        qDebug().noquote() << "Notes       :" << ds.notes;
        qDebug().noquote() << "================================";

        // if (ds.indexed)
        // {
            qDebug() << "Disk Already Indexed:" << ds.filePath;

            IndexDBManager repo(caseManager.getCurrentCase().casePath + "/indexes/disk-" % QString::number(ds.id));

            if (repo.open())
            {
                uint64_t rootId = 0;
                if (repo.getNodeByPath("/", rootId))
                {
                    qInfo() << "Root Node ID:" << rootId;
                    NodeMetadata metadata{};
                    if (repo.getNode(rootId, metadata))
                    {
                        qInfo() << "Root Node Metadata: inode=" << metadata.inode << " size=" << metadata.size << " is_dir=" << (int)metadata.is_dir;
                    }

                    auto children = repo.getChildren(rootId);
                    qInfo() << "Root Node Children Count:" << children.size();
                    for (uint64_t cid : children) {
                        QString childPath;
                        NodeMetadata childMeta{};
                        if (repo.getPathForNode(cid, childPath)) {
                            qInfo() << "  Child Path:" << childPath;
                        }
                        if (repo.getNode(cid, childMeta)) {
                            qInfo() << "  Child Node ID:" << cid
                                    << " inode:" << childMeta.inode
                                    << " size:" << childMeta.size
                                    << " is_dir:" << (int)childMeta.is_dir;
                        } else {
                            qInfo() << "  Child Node ID:" << cid << " (no metadata)";
                        }
                    }
                }
                else
                {
                    qWarning() << "Root Path '/' not Found in LMDB";
                }

                QString partRoot = "2:0:/";
                uint64_t partRootId = 0;
                if (repo.getNodeByPath(partRoot, partRootId))
                {
                    qInfo() << partRoot << "Node ID:" << partRootId;
                    NodeMetadata metadata{};
                    if (repo.getNode(partRootId, metadata))
                    {
                        qInfo() << partRoot << "Metadata: inode=" << metadata.inode << " size=" << metadata.size << " is_dir=" << (int)metadata.is_dir;
                    }
                    auto children = repo.getChildren(partRootId);
                    qInfo() << partRoot << " Children Count:" << children.size();
                    repo.debugListChildrenInDb(partRootId);
                    for (uint64_t cid : children) {
                        QString childPath;
                        NodeMetadata childMeta{};
                        if (repo.getPathForNode(cid, childPath)) {
                            qInfo() << "  Child Path:" << childPath;
                        }
                        if (repo.getNode(cid, childMeta)) {
                            qInfo() << "  Child Node ID:" << cid
                                    << " inode:" << childMeta.inode
                                    << " size:" << childMeta.size
                                    << " is_dir:" << (int)childMeta.is_dir;
                        } else {
                            qInfo() << "  Child Node ID:" << cid << " (no metadata)";
                        }
                    }
                }
                else
                {
                    qWarning() << partRoot << "not Found in LMDB";
                }

                QString bootPath = "2:0:/boot";
                uint64_t bootId = 0;
                if (repo.getNodeByPath(bootPath, bootId))
                {
                    qInfo() << bootPath << "Node ID:" << bootId;
                    NodeMetadata metadata{};
                    if (repo.getNode(bootId, metadata))
                    {
                        qInfo() << bootPath << "Metadata: inode=" << metadata.inode << " size=" << metadata.size << " is_dir=" << (int)metadata.is_dir;
                    }
                    auto children = repo.getChildren(bootId);
                    qInfo() << bootPath << " Children Count:" << children.size();
                    repo.debugListChildrenInDb(bootId);
                    for (uint64_t cid : children) {
                        QString childPath;
                        NodeMetadata childMeta{};
                        if (repo.getPathForNode(cid, childPath)) {
                            qInfo() << "  Child Path:" << childPath;
                        }
                        if (repo.getNode(cid, childMeta)) {
                            qInfo() << "  Child Node ID:" << cid
                                    << " inode:" << childMeta.inode
                                    << " size:" << childMeta.size
                                    << " is_dir:" << (int)childMeta.is_dir;
                        } else {
                            qInfo() << "  Child Node ID:" << cid << " (no metadata)";
                        }
                    }
                }
                else
                {
                    qWarning() << bootPath << "not Found in LMDB";
                }

                repo.close();
            }
            // else
            // {
            //     qWarning() << "Failed to Open FS Index Repository for:" << lmdbPath;
            // }
        // }
        // else
        // {
            // qDebug() << "Disk Requires Ingest:" << ds.filePath;
            // indexManager->CreateJob(ds, caseManager.getCurrentCase().casePath);
        // }
    }
}


