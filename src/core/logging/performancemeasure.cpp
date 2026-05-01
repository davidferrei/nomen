#include "performancemeasure.h"

PerformanceTimer::PerformanceTimer(const QString& text) : description(text)
{
    timer.start();
}

PerformanceTimer::~PerformanceTimer()
{
    qDebug() << "[PERF]" << description << "took" << timer.nsecsElapsed() / 1'000'000.0 << "ms" << "("  << timer.nsecsElapsed() << " ns)";
}
