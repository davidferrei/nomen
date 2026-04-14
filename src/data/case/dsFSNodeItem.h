#ifndef CASETREE_TYPES_H
#define CASETREE_TYPES_H

#include "../index/nodeMetaData.h"

#include <QString>
#include <QVector>

struct DSFSNodeItem {
    quint64 id;
    QString name;
    NodeMetadata metadata;
    QString path;
};

#endif // CASETREE_TYPES_H
