#ifndef LOGGER_H
#define LOGGER_H

#include <QMessageLogContext>
#include <QDebug>

void nomenLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg);

extern QtMessageHandler originalHandler;

#endif // LOGGER_H
