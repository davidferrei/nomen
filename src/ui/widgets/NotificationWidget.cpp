#include "NotificationWidget.h"

QList<NotificationWidget *> NotificationWidget::activeToastsList;

NotificationWidget::NotificationWidget(QWidget *host, const QString &title, const QString &description, int durationMs, NotificationType type) : QFrame(host)
{
    setObjectName(QStringLiteral("NomenToast"));

    const QPalette palette  = QApplication::palette();
    const QColor background = palette.color(QPalette::ToolTipBase);
    const QColor foreground = palette.color(QPalette::ToolTipText);

    const QColor accentColor = [&]() -> QColor 
    {
        switch (type) 
        {
            case NotificationType::Success: 
                return QColor(0x2e, 0xa8, 0x43);
            case NotificationType::Failure: 
                return QColor(0xd9, 0x34, 0x2e);
            default: 
                return palette.color(QPalette::Highlight);
        }
    }();

    const bool isDarkTheme = background.lightness() < 128;
    const QColor headerBackground = isDarkTheme ? background.lighter(130) : background.darker(130);

    const QString iconText = [&]() -> QString 
    {
        switch (type) 
        {
            case NotificationType::Success: 
                return QStringLiteral("OK");
            case NotificationType::Failure: 
                return QStringLiteral("NOK");
            default:            
                return QStringLiteral("Info");
        }
    }();

    const QString iconColorHex = [&]() -> QString 
    {
        switch (type) 
        {
            case NotificationType::Success: 
                return QStringLiteral("#2ea843");
            case NotificationType::Failure: 
                return QStringLiteral("#d9342e");
            default:
                return accentColor.name();
        }
    }();

    const QString borderColorHex = (type == NotificationType::Success) ? accentColor.darker(125).name() : accentColor.name();
        
    //############################################################################################
    // Layout

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Header
    auto *headerWidget = new QWidget(this);
    headerWidget->setObjectName(QStringLiteral("NomenToastHeader"));

    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(14, 8, 8, 8);
    headerLayout->setSpacing(6);

    auto *iconLabel = new QLabel(iconText, headerWidget);
    iconLabel->setObjectName(QStringLiteral("NomenToastIcon"));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    headerLayout->addWidget(iconLabel, 0);

    titleLabel = new QLabel(title, headerWidget);
    titleLabel->setObjectName(QStringLiteral("NomenToastTitle"));
    titleLabel->setWordWrap(false);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    headerLayout->addWidget(titleLabel, 1);

    closeButton = new QPushButton(QStringLiteral("×"), headerWidget);
    closeButton->setObjectName(QStringLiteral("NomenToastClose"));
    closeButton->setFixedSize(22, 22);
    closeButton->setCursor(Qt::ArrowCursor);
    headerLayout->addWidget(closeButton, 0);

    outerLayout->addWidget(headerWidget);

    // Description body (optional)
    if (!description.isEmpty())
    {
        descLabel = new QLabel(description, this);
        descLabel->setObjectName(QStringLiteral("NomenToastDesc"));
        descLabel->setWordWrap(true);
        descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        descLabel->setContentsMargins(14, 8, 14, 10);
        outerLayout->addWidget(descLabel);
    }

    //############################################################################################
    // Stylesheet

    setStyleSheet(QStringLiteral(
        "QFrame#NomenToast {"
        "  background-color: %1;"
        "  border: 2px solid %4;"
        "  border-radius: 10px;"
        "}"
        "QWidget#NomenToastHeader {"
        "  background-color: %2;"
        "  border-top-left-radius: 8px;"
        "  border-top-right-radius: 8px;"
        "  border-bottom-left-radius: 0px;"
        "  border-bottom-right-radius: 0px;"
        "}"
        "QLabel#NomenToastIcon  { color: %6; background: transparent; font-size: 15px; font-weight: bold;}"
        "QLabel#NomenToastTitle { color: %3; background: transparent; font-size: 14px; }"
        "QLabel#NomenToastDesc  { color: %3; background: transparent; font-size: 12px; }"
        "QPushButton#NomenToastClose {"
        "  color: %3; background: transparent; border: none;"
        "  font-size: 16px; font-weight: bold; border-radius: 4px;"
        "}"
        "QPushButton#NomenToastClose:hover { background-color: %4; color: %5; }"
    ).arg(
        background.name(),
        headerBackground.name(),
        foreground.name(),
        borderColorHex,
        palette.color(QPalette::HighlightedText).name(),
        iconColorHex
    ));

    //############################################################################################
    // Animations

    opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.0);
    setGraphicsEffect(opacityEffect);

    auto *fadeInAnim = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeInAnim->setDuration(FADE_DURATION_MS);
    fadeInAnim->setStartValue(0.0);
    fadeInAnim->setEndValue(1.0);
    fadeInAnim->setEasingCurve(QEasingCurve::OutCubic);

    fadeOutAnim = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeOutAnim->setDuration(FADE_DURATION_MS);
    fadeOutAnim->setStartValue(1.0);
    fadeOutAnim->setEndValue(0.0);
    fadeOutAnim->setEasingCurve(QEasingCurve::InCubic);

    connect(fadeOutAnim, &QPropertyAnimation::finished, this, [this]
    {
        activeToastsList.removeOne(this);
        if (parentWidget())
            repositionStack(parentWidget());
        deleteLater();
    });

    //############################################################################################
    // Auto-dismiss Timer

    dismissTimer = new QTimer(this);
    dismissTimer->setSingleShot(true);

    connect(dismissTimer, &QTimer::timeout, this, [this]
    {
        fadeOutAnim->start();
    });

    //############################################################################################
    // Close Button

    connect(closeButton, &QPushButton::clicked, this, [this]
    {
        dismissTimer->stop();
        fadeOutAnim->start();
    });

    //############################################################################################
    // Register, Show, and Kick off Fade-in

    activeToastsList.append(this);
    adjustSize();
    QFrame::show();
    QTimer::singleShot(0, this, [this, fadeInAnim, durationMs]
    {
        repositionStack(parentWidget());
        raise();
        fadeInAnim->start(QAbstractAnimation::DeleteWhenStopped);
        dismissTimer->start(qMax(FADE_DURATION_MS, durationMs - FADE_DURATION_MS));
    });

    if (host)
        host->installEventFilter(this);
}

//############################################################################################
// Logic

void NotificationWidget::show(QWidget *parent, const QString &title, const QString &description, int durationMs, NotificationType type)
{

    QWidget *host = parent;
    if (auto *mainWindow = qobject_cast<QMainWindow *>(parent))
        if (QWidget *centralWidget = mainWindow->centralWidget())
            host = centralWidget;

    NotificationWidget *w = new NotificationWidget(host, title, description, durationMs, type);
    // qDebug() << "[NotificationWidget] instance created:" << w << "parentWidget:" << w->parentWidget() << "activeToasts:" << activeToastsList.size();
}

void NotificationWidget::repositionStack(QWidget *host)
{
    if (!host) 
    {
        qWarning() << "[NotificationWidget] RepositionStack Called with NULL Host";
        return;
    }
    
    QList<NotificationWidget *> hostToasts;
    for (auto *toast : std::as_const(activeToastsList))
        if (toast->parentWidget() == host)
            hostToasts.append(toast);

    // Set Width
    const int toastWidth = qBound(280, host->width() * 22 / 100, 520);
    int nextBottomEdge = host->height() - MARGIN_PX;

    for (int i = hostToasts.size() - 1; i >= 0; --i)
    {
        NotificationWidget *toastInstance = hostToasts[i];
        toastInstance->setFixedWidth(toastWidth);
        toastInstance->adjustSize();

        const int x = host->width() - toastWidth - MARGIN_PX;
        const int y = nextBottomEdge - toastInstance->sizeHint().height();
        nextBottomEdge = y - GAP_PX;

        const QPoint targetPosition(x, y);

        if (!toastInstance->isPositioned)
        {
            toastInstance->move(targetPosition);
            toastInstance->isPositioned = true;
        }
        else if (toastInstance->pos() != targetPosition)
        {

            if (toastInstance->slideAnim)
            {
                toastInstance->slideAnim->stop();
                delete toastInstance->slideAnim;
            }

            auto *slideAnim = new QPropertyAnimation(toastInstance, "pos", toastInstance);
            toastInstance->slideAnim = slideAnim;
            slideAnim->setDuration(180);
            slideAnim->setStartValue(toastInstance->pos());
            slideAnim->setEndValue(targetPosition);
            slideAnim->setEasingCurve(QEasingCurve::OutCubic);

            connect(slideAnim, &QPropertyAnimation::finished, toastInstance, [toastInstance, slideAnim]
            {
                if (toastInstance->slideAnim == slideAnim)
                    toastInstance->slideAnim = nullptr;
            });

            slideAnim->start();
        }
    }
}

//############################################################################################
// Event Filter 

bool NotificationWidget::eventFilter(QObject *watched, QEvent *event)
{
    // Adjust on Resize
    if (watched == parentWidget() && event->type() == QEvent::Resize)
        repositionStack(parentWidget());

    return QFrame::eventFilter(watched, event);
}
