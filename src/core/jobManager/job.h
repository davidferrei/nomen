#ifndef JOB_H
#define JOB_H

#include <QObject>
#include <QUuid>

#include "data/evidence/dataSourceData.h"

class Job : public QObject
{
    Q_OBJECT
public:
    enum class JobType { Index, Ingest };
    enum class JobState { Pending, Running, Paused, Completed, Failed, Cancelled };

    Job(JobType t, const DataSourceData& ds, QObject* parent = nullptr);
    virtual ~Job();

    virtual void start() = 0;
    virtual void requestCancel() { cancelled = true; }

    QString getId() const { return id; }
    JobType getType() const { return jobType; }
    JobState getState() const { return state; }
    QString getDSFilePath() const { return dataSource.filePath; }

signals:
    void started();
    void finished();
    void failed(const QString& error);
    void progress(int percent);

protected:
    QString id;
    JobType jobType;
    JobState state = JobState::Pending;
    bool cancelled = false;
    DataSourceData dataSource;
};

#endif // JOB_H
