#ifndef TIMESTAMPHELPERS_H
#define TIMESTAMPHELPERS_H

#include <ibpp/core/ibpp.h>

#include <QDateTime>
#include <QString>

QDateTime fromIBPPTimestamp(const IBPP::Timestamp &ts);

bool toIBPPTimestamp(const QDateTime &dt, IBPP::Timestamp &out);

bool toIBPPTimestamp(const QString &isoDateStr, IBPP::Timestamp &out);

#endif // TIMESTAMPHELPERS_H
