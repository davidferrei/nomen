#include "logger.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QThread>


QtMessageHandler originalHandler = nullptr;


void nomenLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QThread *currentThread = QThread::currentThread();
    QString threadName = currentThread->objectName();
    QString logType;

    switch (type) {
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
            logType = "Unkown";
            break;
    }

     QString formattedMessage = QString("[%1] [%2] - \"%3\", %4, %5, Thread: \"%6\"")
                                    .arg(dateTime)
                                    .arg(logType)
                                    .arg(localMsg.constData())
                                    .arg(file)
                                    .arg(function)
                                    .arg(threadName.isEmpty() ? "Unkown" : threadName);


    if (type == QtDebugMsg || type == QtInfoMsg) {
        fprintf(stdout, "%s\n", qPrintable(formattedMessage));
        fflush(stdout);
    } else {
        fprintf(stderr, "%s\n", qPrintable(formattedMessage));
    }

    if (originalHandler) {
        originalHandler(type, context, msg);
    }

}

