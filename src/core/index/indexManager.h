#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H

#include <QObject>

#include "data/evidence/dataSourceData.h"

class IndexManager : public QObject
{
    Q_OBJECT

public:
    IndexManager(QObject* parent = nullptr);

};

#endif // INDEXMANAGER_H
