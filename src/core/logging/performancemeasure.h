#ifndef PERFORMANCEMEASURE_H
#define PERFORMANCEMEASURE_H

#include <QString>
#include <QElapsedTimer>
#include <QDebug>

class PerformanceTimer
{
public:
    explicit PerformanceTimer(const QString& text);
    ~PerformanceTimer();

private:
    QString description;
    QElapsedTimer timer;
};

#endif // PERFORMANCEMEASURE_H
