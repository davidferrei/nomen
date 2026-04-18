#include "job.h"

#include <QThread>

Job::~Job() {}

Job::Job(JobType t, const DataSourceData& ds, QObject* parent) : QObject(parent), jobType(t), dataSource(ds)
{
	id = QUuid::createUuid().toString(QUuid::WithoutBraces);
	qDebug() << "[Job] Created" << this << "type:" << static_cast<int>(jobType) << "id:" << id << "file:" << dataSource.filePath;
}


