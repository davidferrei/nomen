#ifndef TSKWALKER_H
#define TSKWALKER_H

#include <tsk/libtsk.h>

#include "../../lmdb/lmdbManager.h"

#include "data/index/nodeMetaData.h"


#include <QString>
#include <QDebug>
#include <QList>

#include <cstring>
#include <vector>

class LMDBManager;

class TskWalker
{
public:
    static bool walkImage(const QString &imagePath, LMDBManager &lmdb);
};

#endif // TSKWALKER_H
