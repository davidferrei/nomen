#ifndef DEBUGLOGGER_H
#define DEBUGLOGGER_H

#include <QMessageLogContext>
#include <QDebug>

void debugLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg);

extern QtMessageHandler originalHandler;

#endif // DEBUGLOGGER_H
