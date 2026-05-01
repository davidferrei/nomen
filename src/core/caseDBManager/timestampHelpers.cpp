#include "timestampHelpers.h"

QDateTime fromIBPPTimestamp(const IBPP::Timestamp &ts)
{
    int rawDate = ts.GetDate();
    if (rawDate < IBPP::MinDate || rawDate > IBPP::MaxDate) 
    {
        qWarning() << "[TimestampHelpers] fromIBPPTimestamp Invalid Date Value:" << rawDate;
        return QDateTime();
    }

    int rawTime = ts.GetTime();
    if (rawTime < 0 || rawTime > 863999999) 
    {
        qWarning() << "[TimestampHelpers] fromIBPPTimestamp Invalid Time Value:" << rawTime;
        rawTime = 0;
    }

    try 
    {
        int y, mo, d; ts.GetDate(y, mo, d);
        int hh, mi, ss; ts.GetTime(hh, mi, ss);
        QDate dt(y, mo, d);
        QTime tt(hh, mi, ss);
        QDateTime qdt(dt, tt);

        if (!qdt.isValid()) 
        {
            qWarning() << "[TimestampHelpers] fromIBPPTimestamp Invalid QDateTime:" << qdt;
            return QDateTime();
        }
        
        return qdt;
    }
    catch (const IBPP::Exception &e) 
    {
        qWarning() << "[TimestampHelpers] fromIBPPTimestamp Unexpected IBPP exception:" << e.ErrorMessage();
        return QDateTime();
    }
    catch (...) 
    {
        qWarning() << "[TimestampHelpers] fromIBPPTimestamp Unknown Exception";
        return QDateTime();
    }
}

bool toIBPPTimestamp(const QDateTime &dt, IBPP::Timestamp &out)
{
    if (!dt.isValid()) 
    { 
        qWarning() << "[TimestampHelpers] toIBPPTimestamp Invalid QDateTime:" << dt;
        return false;
    }

    QDate d = dt.date();
    QTime t = dt.time();

    try 
    {
        out = IBPP::Timestamp(d.year(), d.month(), d.day(), t.hour(), t.minute(), t.second(), t.msec() * 10);
        return true;
    }
    catch (const IBPP::Exception &e) 
    {
        qWarning() << "[TimestampHelpers] toIBPPTimestamp IBPP exception:" << e.ErrorMessage();
        return false;
    }
    catch (...) 
    {
        qWarning() << "[TimestampHelpers] toIBPPTimestamp Unknown Exception";
        return false;
    }
}

bool toIBPPTimestamp(const QString &isoDateStr, IBPP::Timestamp &out)
{
    QDateTime dt = QDateTime::fromString(isoDateStr, Qt::ISODate);

    if (!dt.isValid()) 
    { 
        qWarning() << "[TimestampHelpers] toIBPPTimestamp Invalid QDateTime:" << dt;
        return false;
    }

    return toIBPPTimestamp(dt, out);
}
