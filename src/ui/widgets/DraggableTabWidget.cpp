#include "DraggableTabWidget.h"
#include "DraggableTabBar.h"



DraggableTabWidget::DraggableTabWidget(QWidget *parent) : QTabWidget(parent)
{
    setTabBar(new DraggableTabBar(this));
    setAcceptDrops(true);
}

void DraggableTabWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-tabwidget-tab"))
        event->acceptProposedAction();
    else
        QTabWidget::dragEnterEvent(event);
}

void DraggableTabWidget::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("application/x-tabwidget-tab")) 
    {
        QTabWidget::dropEvent(event);
        return;
    }

    QByteArray tabInfoArray = event->mimeData()->data("application/x-tabwidget-tab");
    QDataStream dataStream(&tabInfoArray, QIODevice::ReadOnly);
    quintptr srcWidgetPtr;
    int srcTabIndex;
    dataStream >> srcWidgetPtr >> srcTabIndex;
    QTabWidget *srcTabWidget = reinterpret_cast<QTabWidget*>(srcWidgetPtr);
    if (!srcTabWidget) return;

    if (srcTabWidget == this) return;

    QWidget *pageWidget = srcTabWidget->widget(srcTabIndex);
    if (!pageWidget) return;

    QString tabText = srcTabWidget->tabText(srcTabIndex);
    QIcon tabIcon = srcTabWidget->tabIcon(srcTabIndex);

    srcTabWidget->removeTab(srcTabIndex);
    this->addTab(pageWidget, tabIcon, tabText);
    this->setCurrentWidget(pageWidget);

    event->acceptProposedAction();
}
