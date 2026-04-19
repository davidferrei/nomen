#include "logger.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QThread>


#define ANSI_RESET "\033[0m"
#define ANSI_RED "\033[31m"
#define ANSI_CYAN "\033[36m"
#define ANSI_BLUE "\033[34m"
#define ANSI_YELLOW "\033[33m"


QtMessageHandler originalHandler = nullptr;

static QFile g_logFile;
static QTextStream g_logStream;


void initLogger()
{
    QString dateForFile = QDateTime::currentDateTime().toString("dd_MM_yyyy");
    QString logFilePath = QString("/tmp/nomen_%1.log").arg(dateForFile);

    g_logFile.setFileName(logFilePath);

    if (g_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        g_logStream.setDevice(&g_logFile);
        g_logStream << "===== Nomen =====\n";
        g_logStream.flush();
    } 
    else 
    {
        qCritical() << "Failed To Open Temp Log File At:" << logFilePath;
    }

    originalHandler = qInstallMessageHandler(nomenLogger);
}

void shutdownLogger()
{
    if (g_logFile.isOpen()) 
    {
        g_logStream << "===== Bye =====\n";
        g_logStream.flush();
        g_logFile.close();
    }
}

void setLogFilePath(const QString &path)
{
    if (g_logFile.isOpen())
        g_logFile.close();

    g_logFile.setFileName(path);

    if (g_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        g_logStream.setDevice(&g_logFile);
        g_logStream << "===== Nomen =====\n";
        g_logStream.flush();
    } 
    else 
    {
        qCritical() << "Failed To Open Log File At:" << path;
    }
}

void nomenLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    //TODO: In Release Build return when type is Debug, Better yet have a "Enable Debug Mode"

    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QThread *currentThread = QThread::currentThread();
    QString threadName = currentThread->objectName();

    QString logType;
    switch (type) 
    {
        case QtInfoMsg:
            logType = "INFO";
            break;
        case QtDebugMsg:
            logType = "DEBUG";
            break;
        case QtWarningMsg:
            logType = "WARNING";
            break;
        case QtCriticalMsg:
            logType = "CRITICAL";
            break;
        case QtFatalMsg:
            logType = "FATAL";
            break;
        default:
            logType = "UNKNOWN";
            break;
    }

    QString formattedMessage =
        QString("[%1] [%2] - \"%3\", %4, %5, Thread: \"%6\"")
            .arg(dateTime,
                logType,
                QString::fromUtf8(localMsg),
                QString::fromUtf8(file),
                QString::fromUtf8(function),
                threadName.isEmpty() ? QStringLiteral("Unknown") : threadName);


#ifdef QT_DEBUG
    const char* color;

    switch (type) 
    {
        case QtDebugMsg: 
            color = ANSI_CYAN; 
            break;
        case QtInfoMsg: 
            color = ANSI_BLUE; 
            break;
        case QtWarningMsg: 
            color = ANSI_YELLOW; 
            break;
        case QtCriticalMsg:
        case QtFatalMsg: 
            color = ANSI_RED; 
            break;
        default: 
            color = ANSI_RESET; 
            break;
    }

    // Console Output
    if (type == QtDebugMsg || type == QtInfoMsg) {
        fprintf(stdout, "%s%s%s\n", color, qPrintable(formattedMessage), ANSI_RESET);
        fflush(stdout);
    } else 
    {
        fprintf(stderr, "%s%s%s\n", color, qPrintable(formattedMessage), ANSI_RESET);
    }
#endif

    // File Output
    if (g_logFile.isOpen()) 
    {
        g_logStream << formattedMessage << "\n";
        g_logStream.flush();
    }

    if (originalHandler) 
    {
        originalHandler(type, context, msg);
    }
}

void qDebugNL()
{
#ifdef QT_DEBUG
    fprintf(stdout, "\n");
    fflush(stdout);
#endif

    if (g_logFile.isOpen()) 
    {
        g_logStream << "\n";
        g_logStream.flush();
    }
}
