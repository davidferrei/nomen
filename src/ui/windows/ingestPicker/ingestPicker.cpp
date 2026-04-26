#include "ingestPicker.h"
#include "ui_ingestPicker.h"

ingestPicker::ingestPicker(QWidget *parent) : QMainWindow(parent), ui(new Ui::ingestPicker)
{
    ui->setupUi(this);
}

ingestPicker::~ingestPicker()
{
    delete ui;
}

//############################################################################################
//UI Stuff
