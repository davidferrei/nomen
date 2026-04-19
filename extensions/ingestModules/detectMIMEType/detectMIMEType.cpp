#include "detectMIMEType.h"

void DetectMIMETypeModule::process()
{
    for (int i = 1; i <= 10; ++i) 
    {
        qDebug() << "[IngestModule][DetectMIMEType] Number" << i;
        QThread::sleep(1);
    }
}
