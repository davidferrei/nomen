#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent, CaseManager& caseManager) : QMainWindow(parent), ui(new Ui::MainWindow), caseManager(caseManager)
{
    ui->setupUi(this);

    qDebug() << "Main Window Opened";

    setupUiElements();

    plainTextViewer = new PlainTextViewer(ui->plainTextView, ui->plainTextView);
    metadataViewer  = new MetadataViewer(ui->metadataViewerTab, this);
    

    //################################
    // Jobs Manager (Index/Ingest)

    jobManager = new JobManager(this);

    setupJobSignalConnections();  

    loadDataSources();

    setupUiSignalConnections();

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
        int idxContent = ui->MainTabGroup->indexOf(ui->contentViewerTab);
        if (idxContent >= 0 && ui->MainTabGroup->tabIcon(idxContent).isNull())
        {
            QIcon contentIc = QIcon::fromTheme(QStringLiteral("document-open"));
            ui->MainTabGroup->setTabIcon(idxContent, contentIc);
        }

        int idxStats = ui->MainTabGroup->indexOf(ui->statsViewerTab_2);
        if (idxStats >= 0 && ui->MainTabGroup->tabIcon(idxStats).isNull())
        {
            QIcon statsIc = QIcon::fromTheme(QStringLiteral("view-statistics"));
            ui->MainTabGroup->setTabIcon(idxStats, statsIc);
        }

        DraggableTabWidget *newMain = new DraggableTabWidget(this);
        while (ui->MainTabGroup->count() > 0)
        {
            QWidget *w = ui->MainTabGroup->widget(0);
            QString t = ui->MainTabGroup->tabText(0);
            QIcon ic = ui->MainTabGroup->tabIcon(0);
            ui->MainTabGroup->removeTab(0);
            newMain->addTab(w, ic, t);
        }
        QSplitter *h2 = ui->horizontalSplitter;
        if (h2)
        {
            int idx = h2->indexOf(ui->MainTabGroup);
            if (idx >= 0) h2->replaceWidget(idx, newMain);
            else h2->addWidget(newMain);
        }
    }

    if (ui->BottomTabGroup)
        {
        int idxFs = ui->BottomTabGroup->indexOf(ui->fsExplorerTab);
        if (idxFs >= 0 && ui->BottomTabGroup->tabIcon(idxFs).isNull())
        {
            QIcon fsIc = QIcon::fromTheme(QStringLiteral("folder"));
            ui->BottomTabGroup->setTabIcon(idxFs, fsIc);
        }

        int idxMeta = ui->BottomTabGroup->indexOf(ui->metadataViewerTab);
        if (idxMeta >= 0 && ui->BottomTabGroup->tabIcon(idxMeta).isNull())
        {
            QIcon metaIc = QIcon::fromTheme(QStringLiteral("dialog-information"));
            ui->BottomTabGroup->setTabIcon(idxMeta, metaIc);
        }

        DraggableTabWidget *newBottom = new DraggableTabWidget(this);
        while (ui->BottomTabGroup->count() > 0)
        {
            QWidget *w = ui->BottomTabGroup->widget(0);
            QString t = ui->BottomTabGroup->tabText(0);
            QIcon ic = ui->BottomTabGroup->tabIcon(0);
            ui->BottomTabGroup->removeTab(0);
            newBottom->addTab(w, ic, t);
        }
        QSplitter *v2 = ui->verticalSplitter;
        if (v2)
        {
            int idx = v2->indexOf(ui->BottomTabGroup);
            if (idx >= 0) v2->replaceWidget(idx, newBottom);
            else v2->addWidget(newBottom);
        }
    }

    if (ui->treeViewFilesystem)
    {
        DraggableTabWidget *treeTab = new DraggableTabWidget(this);
        QIcon ic = QIcon::fromTheme(QStringLiteral("repository"));
        QString title = QStringLiteral("Case Tree");

        QSplitter *h = ui->horizontalSplitter;
        if (h)
        {
            int idx = h->indexOf(ui->treeViewFilesystem);
            if (idx >= 0)
                h->replaceWidget(idx, treeTab);
            else
                h->addWidget(treeTab);
        }

        treeTab->addTab(ui->treeViewFilesystem, ic, title);
    }

}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    setUIWidgetsSizes();

    if (ui->UpPushButton)
        connect(ui->UpPushButton, &QPushButton::clicked, this, &MainWindow::onFSUpPushButtonClicked);
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


    // Viewers Tab Widget Styling
    QTabWidget* viewers = ui->ViewersTabWidget;
    if (viewers) 
    {
        viewers->setStyleSheet(R"(
            QTabWidget#ViewersTabWidget QTabBar::tab 
            {
                padding: 6px 10px;
                margin-right: 6px;
                border-top-left-radius: 6px;
                border-top-right-radius: 6px;
            }
            QTabWidget#ViewersTabWidget QTabBar::tab:selected 
            {
                background: palette(button);
                color: palette(buttonText);
            }
            QTabWidget#ViewersTabWidget QTabBar::tab:!selected 
            {
                background: transparent;
                color: palette(windowText);
            })"
        );
    }


    
}

//############################################################################################
// Logic

void MainWindow::loadDataSources()
{
    QList<DataSourceData> dataSources = caseManager.getDataSources();

    qDebug() << "Total Disks Found:" << dataSources.size();

    caseTreeModel = new CaseTreeModel(this);
    itemTableModel = new ItemTableModel(caseTreeModel, this);
    itemTableProxy = new QSortFilterProxyModel(this);
    itemTableProxy->setSourceModel(itemTableModel);
    itemTableProxy->setSortRole(Qt::UserRole);

    for (const DataSourceData& ds : dataSources)
    {
        qDebug().noquote() << "========== DataSource ==========";
        qDebug().noquote() << "ID          :" << ds.id;
        qDebug().noquote() << "Name        :" << ds.name;
        qDebug().noquote() << "File Path   :" << ds.filePath;
        qDebug().noquote() << "Indexed     :" << (ds.indexed ? "YES" : "NO");
        qDebug().noquote() << "================================";

        dataSourceNames.insert(static_cast<int>(ds.id), ds.name.isEmpty() ? QStringLiteral("Disk %1").arg(ds.id) : ds.name);

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
            qDebug() << "Disk Requires Indexing:" << ds.filePath;
            jobManager->CreateIndexJob(ds, caseManager.getCurrentCase().casePath);
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

    ui->tableView->setModel(itemTableProxy);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setDragEnabled(false);
    ui->tableView->setDragDropMode(QAbstractItemView::NoDragDrop);

    ui->tableView->verticalHeader()->setVisible(true);

    QHeaderView *th = ui->tableView->horizontalHeader();
    th->setStretchLastSection(false);
    th->setSectionResizeMode(QHeaderView::ResizeToContents);
    th->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableView->setColumnWidth(0, 24);
    th->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableView->setSortingEnabled(true);

    // Test
        DataSourceData t1;
        t1.name = "Hardcoded Test Disk 1";
        t1.filePath = "/mnt/disk01/DiskImage/kali.raw";
        jobManager->CreateIngestJob(t1, caseManager.getCurrentCase().casePath);

        DataSourceData t2;
        t2.name = "Hardcoded Test Disk 2";
        t2.filePath = "/mnt/disk01/DiskImage/kali.raw";
        jobManager->CreateIngestJob(t2, caseManager.getCurrentCase().casePath);

}


//############################################################################################
// Signals

void MainWindow::setupJobSignalConnections()
{
    connect(jobManager, &JobManager::jobStarted, this, [this](const QString& filePath, Job::JobType type)
    {
        qDebug() << "[MainWindow] jobStarted signal received for" << filePath << "type:" << static_cast<int>(type);
        bool isIngest = (type == Job::JobType::Ingest);
        NotificationWidget::show(this,
                                 isIngest ? QStringLiteral("Ingest Started") : QStringLiteral("Indexing Started"),
                                 QFileInfo(filePath).fileName(),
                                 3500,
                                 NotificationWidget::NotificationType::Info);
    });

    connect(jobManager, &JobManager::jobFinished, this, [this](const QString& filePath, Job::JobType type)
    {
        qDebug() << "[MainWindow] jobFinished signal received for" << filePath << "type:" << static_cast<int>(type);
        bool isIngest = (type == Job::JobType::Ingest);
        NotificationWidget::show(this,
                                 isIngest ? QStringLiteral("Ingest Complete") : QStringLiteral("Indexing Complete"),
                                 QFileInfo(filePath).fileName(),
                                 4000,
                                 NotificationWidget::NotificationType::Success);

        if (isIngest) return; 

        const QList<DataSourceData> allDataSources = caseManager.getDataSources();
        const DataSourceData* ds = nullptr;
        for (const DataSourceData& d : allDataSources)
            if (d.filePath == filePath) { ds = &d; break; }

        if (!ds)
        {
            qWarning() << "[MainWindow]: Index Job Finished but no Matching Data Source Found for File Path:" << filePath;
            return;
        }

        QString label = ds->name.isEmpty() ? QStringLiteral("Disk %1").arg(ds->id) : ds->name;
        dataSourceNames.insert(static_cast<int>(ds->id), label);

        QString lmdbPath = caseManager.getCurrentCase().casePath + "/indexes/disk-" + QString::number(ds->id);
        qDebug() << "[MainWindow] Indexing Finished, Adding DS to Tree:" << label;
        caseTreeModel->addDataSource(static_cast<int>(ds->id), label, lmdbPath);
    });

    connect(jobManager, &JobManager::jobFailed, this, [this](const QString& filePath, const QString& error, Job::JobType type)
    {
        qDebug() << "[MainWindow] jobFailed signal received for" << filePath << ":" << error << "type:" << static_cast<int>(type);
        bool isIngest = (type == Job::JobType::Ingest);
        NotificationWidget::show(this,
                                 isIngest ? QStringLiteral("Ingest Failed") : QStringLiteral("Indexing Failed"),
                                 QFileInfo(filePath).fileName() + QStringLiteral(": ") + error,
                                 10000,
                                 NotificationWidget::NotificationType::Failure);
    });
}

void MainWindow::setupUiSignalConnections()
{
    // Case Tree
    connect(caseTreeModel, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex&, int, int){
        ui->treeViewFilesystem->resizeColumnToContents(0);
    });

    connect(ui->treeViewFilesystem, &QTreeView::clicked, caseTreeModel, &CaseTreeModel::onItemActivated);
    connect(caseTreeModel, &CaseTreeModel::nodeActivated, this, &MainWindow::onCaseTreeNodeActivated);

    // FS Explorer Table
    connect(ui->tableView, &QTableView::clicked, this, &MainWindow::onFSTableRowClicked);
    connect(ui->tableView, &QTableView::doubleClicked, this, &MainWindow::onFSTableRowDoubleClicked);


    // Plain Text Viewer & File Metadata
    connect(ui->tableView, &QTableView::clicked, this, [this](const QModelIndex &proxyIndex)
    {
        if (!proxyIndex.isValid())
        {
            qWarning() << "[MainWindow] Invalid Index Clicked in Table";
            return;
        }

        QModelIndex sourceIndex = itemTableProxy->mapToSource(proxyIndex);
        if (!sourceIndex.isValid())
        {
            qWarning() << "[MainWindow] Invalid Source Index Mapped from Proxy Index";
            return;
        }

        DSFSNodeItem item = itemTableModel->itemAt(sourceIndex.row());
        if (item.metadata.is_dir) return;

        QString diskImagePath;
        const QList<DataSourceData> dataSourcesList = caseManager.getDataSources();
        for (const DataSourceData& ds : dataSourcesList)
        {
            if (static_cast<qint64>(ds.id) == navigationSelectedDSId)
            {
                diskImagePath = ds.filePath;
                break;
            }
        }

        plainTextViewer->fileSelected(diskImagePath, item.path);
        metadataViewer->fileSelected(diskImagePath, item.path);

        QString dsName = dataSourceNames.value(navigationSelectedDSId, QStringLiteral("Disk %1").arg(navigationSelectedDSId));
        ui->filePath->setText(QStringLiteral("%1 | %2").arg(dsName, item.path));
    });

    connect(plainTextViewer, &PlainTextViewer::prevPageEnabled, ui->prevPageBtn, &QPushButton::setEnabled);
    connect(plainTextViewer, &PlainTextViewer::nextPageEnabled, ui->nextPageBtn, &QPushButton::setEnabled);
    connect(plainTextViewer, &PlainTextViewer::pageLabelChanged, ui->pageLabel, &QLabel::setText);
    connect(ui->prevPageBtn, &QPushButton::clicked, plainTextViewer, &PlainTextViewer::prevPage);
    connect(ui->nextPageBtn, &QPushButton::clicked, plainTextViewer, &PlainTextViewer::nextPage);


    // Metadata viewer
    connect(metadataViewer, &MetadataViewer::metadataReady, this, [this](const MetadataViewer::FileMetaFileMetadataDisplayStruct& fileMetadata)
    {
        ui->addrLabel->setText(fileMetadata.addr);
        ui->typeLabel->setText(fileMetadata.type);
        ui->flagsLabel->setText(fileMetadata.flags);
        ui->modeLabel->setText(fileMetadata.mode);
        ui->sizeLabel->setText(fileMetadata.size);
        ui->uidLabel->setText(fileMetadata.uid);
        ui->gidLabel->setText(fileMetadata.gid);
        ui->nlinkLabel->setText(fileMetadata.nlink);
        ui->seqLabel->setText(fileMetadata.seq);
        ui->mtimeLabel->setText(fileMetadata.mtime);
        ui->atimeLabel->setText(fileMetadata.atime);
        ui->ctimeLabel->setText(fileMetadata.ctime);
        ui->crtimeLabel->setText(fileMetadata.crtime);
        ui->contentLenLabel->setText(fileMetadata.contentLen);
        ui->nameLabel->setText(fileMetadata.name);
        ui->shrtNameLabel->setText(fileMetadata.shrtName);
        ui->parAddrLabel->setText(fileMetadata.parAddr);
    });

    

}


void MainWindow::onCaseTreeNodeActivated(int dsId, const QString &path)
{
    if (path.isEmpty())
    {
        qDebug() << "[MainWindow] Signal - nodeActivated -> DS:" << dsId << "No Path Found";
        return;
    }

    qDebug() << "[MainWindow] Signal - nodeActivated -> DS:" << dsId << "Path:" << path;

    auto nodeChildrenResponse = caseTreeModel->getChildrenItemsForDSPath(dsId, path);
    if (!nodeChildrenResponse.first)
    {
        //TODO: Clear Table and Show "No Results" Message
        qDebug() << "[MainWindow] no Results for Path:" << path;
        // ui->currentNodeLabel->setText(QString());
        return;
    }

    QString dsName = dataSourceNames.value(dsId, QStringLiteral("Disk %1").arg(dsId));
    ui->currentNodeLabel->setText(QStringLiteral("%1 | %2").arg(dsName, path));

    navigationSelectedDSId = dsId;
    navigationCurrentPath = path;

    itemTableModel->setRoot(navigationSelectedDSId, navigationCurrentPath);
}

void MainWindow::onFSTableRowClicked(const QModelIndex &proxyIndex)
{
    if (!proxyIndex.isValid()) return;

    QModelIndex sourceIndex = itemTableProxy->mapToSource(proxyIndex);
    if (!sourceIndex.isValid()) return;

    DSFSNodeItem item = itemTableModel->itemAt(sourceIndex.row());
    if (item.metadata.is_dir) return;

    qDebug() << "[MainWindow] Selected File:" << item.path;
}

void MainWindow::onFSTableRowDoubleClicked(const QModelIndex &proxyIndex)
{
    if (!proxyIndex.isValid()) return;

    QModelIndex sourceIndex = itemTableProxy->mapToSource(proxyIndex);
    if (!sourceIndex.isValid()) return;

    DSFSNodeItem item = itemTableModel->itemAt(sourceIndex.row());
    if (!item.metadata.is_dir) return;

    int dsId = navigationSelectedDSId;
    if (dsId < 0 || item.path.isEmpty()) return;

    QString parentPath = navigationCurrentPath;

    navigationCurrentPath = item.path;
    QString dsName = dataSourceNames.value(dsId, QStringLiteral("Disk %1").arg(dsId));
    ui->currentNodeLabel->setText(QStringLiteral("%1 | %2").arg(dsName, item.path));
    itemTableModel->setRoot(dsId, item.path);

    auto parentNodeResult = caseTreeModel->getNodeIdForPath(dsId, parentPath);
    if (parentNodeResult.first)
    {
        QModelIndex parentTreeIdx = caseTreeModel->findIndex(dsId, parentNodeResult.second);
        if (parentTreeIdx.isValid())
        {
            ui->treeViewFilesystem->expand(parentTreeIdx);

            QModelIndex targetTreeIdx = caseTreeModel->findIndex(dsId, item.id);
            if (targetTreeIdx.isValid())
            {
                ui->treeViewFilesystem->setCurrentIndex(targetTreeIdx);
                ui->treeViewFilesystem->scrollTo(targetTreeIdx);
            }
        }
    }
}


void MainWindow::onFSUpPushButtonClicked()
{
    if (navigationSelectedDSId < 0 || navigationCurrentPath.isEmpty())
    {
        qWarning() << "[MainWindow] UpPushButton: No Current Selection";
        return;
    }


    int firstSlash = navigationCurrentPath.indexOf('/');
    if (firstSlash < 0)
    {
        //FIXME: Does not Work on Special Nodes
        qWarning() << "[MainWindow] UpPushButton: Malformed Path (No Slash):" << navigationCurrentPath;
        return;
    }

    const QString prefix = navigationCurrentPath.left(firstSlash);
    const QString fsPath = navigationCurrentPath.mid(firstSlash);

    if (fsPath == QStringLiteral("/"))
    {
        qWarning() << "[MainWindow] UpPushButton: Already at Filesystem Root";
        return;
    }

    
    int lastSlash = fsPath.lastIndexOf('/');
    QString parentFsPath = (lastSlash <= 0) ? QStringLiteral("/") : fsPath.left(lastSlash);
    QString parentPath = prefix + parentFsPath;

    QString dsName = dataSourceNames.value(navigationSelectedDSId, QStringLiteral("Disk %1").arg(navigationSelectedDSId));
    ui->currentNodeLabel->setText(QStringLiteral("%1 | %2").arg(dsName, parentPath));

    navigationCurrentPath = parentPath;
    itemTableModel->setRoot(navigationSelectedDSId, parentPath);
}


