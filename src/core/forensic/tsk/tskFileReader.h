#ifndef TSKFILEREADER_H
#define TSKFILEREADER_H

#include <tsk/libtsk.h>

#include <unordered_map>

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QDebug>

struct TskFileMetadata
{
    QString  fsPath;

    quint64 addr = 0;
    int type = 0;   // TSK_FS_META_TYPE_ENUM
    int flags = 0;  // TSK_FS_META_FLAG_ENUM
    int mode = 0;   // TSK_FS_META_MODE_ENUM
    qint64 size = 0;
    quint32 uid = 0;
    quint32 gid = 0;
    int nlink = 0;
    quint32 seq = 0;

    qint64 mtime = 0;  
    quint32 mtime_nano = 0;
    qint64 atime = 0;  
    quint32 atime_nano = 0;
    qint64 ctime = 0;  
    quint32 ctime_nano = 0;
    qint64 crtime = 0;  
    quint32 crtime_nano = 0;

    qint64 contentLen = 0;

    bool hasName = false;
    QString name;
    QString shrtName;
    quint64 parAddr = 0;
};

class tskFileReader : public QObject
{
    Q_OBJECT
public:
    explicit tskFileReader(QObject* parent = nullptr);
    ~tskFileReader();

    QByteArray readFileContentFromImage(const QString& diskImagePath, const QString& fsPath, qint64 offset, qint64 length, qint64& fileSize, QString& errorStringOutput);
    void readFileMetadataFromImage(const QString& diskImagePath, const QString& fsPath, QString& errorStringOutput);

signals:
    void fileOpened(const TskFileMetadata& metadata);

};

#endif // TSKFILEREADER_H
