#ifndef METADATAVIEWER_H
#define METADATAVIEWER_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QDateTime>

#include "core/forensic/tsk/tskFileReader.h"

class MetadataViewer : public QObject
{
    Q_OBJECT
public:
    struct FileMetaFileMetadataDisplayStruct
    {
        QString addr;
        QString type;
        QString flags;
        QString mode;
        QString size;
        QString uid;
        QString gid;
        QString nlink;
        QString seq;
        QString mtime;
        QString atime;
        QString ctime;
        QString crtime;
        QString contentLen;
        QString name;
        QString shrtName;
        QString parAddr;
    };

    explicit MetadataViewer(QWidget* container = nullptr, QObject* parent = nullptr);

public slots:
    void fileSelected(const QString& diskImagePath, const QString& fsPath);

signals:
    void metadataReady(const FileMetaFileMetadataDisplayStruct& display);
};

#endif // METADATAVIEWER_H
