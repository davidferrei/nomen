#ifndef LOGGER_H
#define LOGGER_H

#include <QMessageLogContext>
#include <QDebug>

void initLogger();
void shutdownLogger();
void setLogFilePath(const QString &path);
void nomenLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg);

extern QtMessageHandler originalHandler;

#endif // LOGGER_H
