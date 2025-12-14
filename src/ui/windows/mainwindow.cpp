#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "core/forensic/tsk/tskwalker.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug() << "Main Window Opened";

    //qInfo() << "Start Disk Walk";
    //TskWalker::walkImage("/mnt/disk01/DiskImage/kali.raw");
}

MainWindow::~MainWindow()
{
    delete ui;
}
