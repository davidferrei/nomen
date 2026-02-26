#ifndef DATASOURCEDATA_H
#define DATASOURCEDATA_H

#include <QDate>
#include <QString>

//TODO: Create Evidence Struct and have stuff like Serial Number, Device Model, acquisitionTime

struct DataSourceData {
    QString name;
    quint64 diskSize;
    quint32 sectorSize;
    QString hash;
    QString hashType;
    QString format;
    QString filePath;
    QString timeZone;
    QString notes;
    bool indexed = false;
};


#endif // DATASOURCEDATA_H
