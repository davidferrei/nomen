#ifndef LOGGER_H
#define LOGGER_H

#include <QMessageLogContext>
#include <QDebug>

void initLogger();
void nomenLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void shutdownLogger();

extern QtMessageHandler originalHandler;

#endif // LOGGER_H
