#include "DraggableTabBar.h"


DraggableTabBar::DraggableTabBar(QWidget *parent) : QTabBar(parent)
{
    setAcceptDrops(true);
    setMovable(false);
}

void DraggableTabBar::mousePressEvent(QMouseEvent *event)
{
    pressPos = event->pos();
    pressTabIndex = tabAt(pressPos);
    QTabBar::mousePressEvent(event);
}

void DraggableTabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (pressTabIndex < 0) { QTabBar::mouseMoveEvent(event); return; }

    if ((event->pos() - pressPos).manhattanLength() < QApplication::startDragDistance()) 
    {
        QTabBar::mouseMoveEvent(event);
        return;
    }

    QTabWidget *sourceTabWidget = nullptr;
    QObject *obj = parent();
    while (obj && !(sourceTabWidget = qobject_cast<QTabWidget*>(obj))) obj = obj->parent();
    if (!sourceTabWidget) return;

    QWidget *pageWidget = sourceTabWidget->widget(pressTabIndex);
    if (!pageWidget) return;

    QMimeData *mime = new QMimeData;
    QByteArray tabInfoArray;
    QDataStream dataStream(&tabInfoArray, QIODevice::WriteOnly);
    dataStream << reinterpret_cast<quintptr>(sourceTabWidget) << pressTabIndex;
    mime->setData("application/x-tabwidget-tab", tabInfoArray);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->exec(Qt::MoveAction);
}

void DraggableTabBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-tabwidget-tab"))
        event->acceptProposedAction();
}

void DraggableTabBar::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("application/x-tabwidget-tab")) return;

    QByteArray tabInfoArray = event->mimeData()->data("application/x-tabwidget-tab");
    QDataStream dataStream(&tabInfoArray, QIODevice::ReadOnly);
    quintptr srcWidgetPtr;
    int srcTabIndex;
    dataStream >> srcWidgetPtr >> srcTabIndex;

    QTabWidget *srcTabWidget = reinterpret_cast<QTabWidget*>(srcWidgetPtr);
    if (!srcTabWidget) return;

    QWidget *pageWidget = srcTabWidget->widget(srcTabIndex);
    if (!pageWidget) return;

    QString tabText = srcTabWidget->tabText(srcTabIndex);
    QIcon tabIcon = srcTabWidget->tabIcon(srcTabIndex);

    srcTabWidget->removeTab(srcTabIndex);

    QTabWidget *destTabWidget = nullptr;
    QObject *obj = parent();
    while (obj && !(destTabWidget = qobject_cast<QTabWidget*>(obj))) obj = obj->parent();
    
    if (!destTabWidget) return;

    destTabWidget->addTab(pageWidget, tabIcon, tabText);
    destTabWidget->setCurrentWidget(pageWidget);

    event->acceptProposedAction();
}
