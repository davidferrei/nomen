#ifndef DRAGGABLETABWIDGET_H
#define DRAGGABLETABWIDGET_H

#include <QTabWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDataStream>
#include <QByteArray>

class DraggableTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit DraggableTabWidget(QWidget *parent = nullptr);
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // DRAGGABLETABWIDGET_H
