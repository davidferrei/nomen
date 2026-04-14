#include "job.h"

Job::~Job() {}

Job::Job(JobType t, const DataSourceData& ds, QObject* parent) : QObject(parent), jobType(t), dataSource(ds)
{
	id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}
