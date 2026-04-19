#include "txtCounterModule.h"

void TxtCounterModule::process()
{
	for (int i = 1; i <= 10; ++i)
	{
		qDebug() << "[IngestModule][TxtCounterModule] Number" << i;
		QThread::sleep(1);
	}
}


