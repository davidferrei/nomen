#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QFrame>
#include <QList>
#include <QApplication>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPalette>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>

// class QLabel;
// class QPushButton;
// class QGraphicsOpacityEffect;
// class QPropertyAnimation;
// class QTimer;

class NotificationWidget : public QFrame
{
    Q_OBJECT

public:
    enum class NotificationType 
    { 
        Info, 
        Success, 
        Failure 
    };

    static void show(QWidget *parent, const QString &title, const QString &description = {}, int durationMs = 3500, NotificationType type = NotificationType::Info);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    explicit NotificationWidget(QWidget *host, const QString &title, const QString &description, int durationMs, NotificationType type);

    static void repositionStack(QWidget *host);

    static QList<NotificationWidget *> activeToastsList;

    // Widgets
    QLabel *titleLabel  = nullptr;
    QLabel *descLabel   = nullptr;
    QPushButton *closeButton = nullptr;

    // Animation  
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    QPropertyAnimation *fadeOutAnim = nullptr;
    QPropertyAnimation *slideAnim = nullptr;
  

    // Lifetime
    QTimer *dismissTimer = nullptr;
    bool isPositioned = false;

    // Shared State

    static constexpr int FADE_DURATION_MS = 220;
    static constexpr int MARGIN_PX = 16;
    static constexpr int GAP_PX = 8;
};

#endif // NOTIFICATIONWIDGET_H
