#ifndef INGESTPICKER_H
#define INGESTPICKER_H

#include <QMainWindow>

namespace Ui {
class ingestPicker;
}

class ingestPicker : public QMainWindow
{
    Q_OBJECT

public:
    explicit ingestPicker(QWidget *parent = nullptr);
    ~ingestPicker();

private:
    Ui::ingestPicker *ui;
};

#endif // INGESTPICKER_H
