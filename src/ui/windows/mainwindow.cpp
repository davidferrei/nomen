#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent, const CaseManager& caseManager)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , caseManager(caseManager)
{
    ui->setupUi(this);

    qDebug() << "Main Window Opened";

    //qInfo() << "Start Disk Walk";
    //TskWalker::walkImage("/mnt/disk01/DiskImage/kali.raw");

    // Action Bar
    QMenu* fileMenu = ui->menubar->addMenu("&File");
    QAction* openAction = new QAction("Open", this);
    QAction* exitAction = new QAction("Exit", this);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(openAction);
    fileMenu->addAction(exitAction);

    // Help Menu
    QMenu* helpMenu = ui->menubar->addMenu("&Help");
    QAction* aboutAction = new QAction("About", this);
    connect(aboutAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "About", "Nomen v1.0");
    });
    helpMenu->addAction(aboutAction);

    // Status Bar
    ui->statusbar->showMessage("Ready");
    QLabel* userLabel = new QLabel("Case: " + caseManager.getCurrentCase().caseName + " | " + caseManager.getCurrentCase().caseId , this);
    ui->statusbar->addPermanentWidget(userLabel);

}

MainWindow::~MainWindow()
{
    delete ui;
}


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
