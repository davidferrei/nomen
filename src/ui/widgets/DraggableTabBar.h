
#ifndef DRAGGABLETABBAR_H
#define DRAGGABLETABBAR_H

#include <QTabBar>
#include <QApplication>
#include <QPoint>
#include <QDrag>
#include <QMimeData>
#include <QDataStream>
#include <QTabWidget>
#include <QWidget>
#include <QByteArray>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QIODevice>
#include <QIcon>

class QMouseEvent;
class QDragEnterEvent;
class QDropEvent;
class QTabWidget;

class DraggableTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit DraggableTabBar(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QPoint pressPos;
    int pressTabIndex = -1;
};

#endif // DRAGGABLETABBAR_H
