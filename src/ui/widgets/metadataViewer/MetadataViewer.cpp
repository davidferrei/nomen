#include "MetadataViewer.h"

MetadataViewer::MetadataViewer(QWidget* /*container*/, QObject* parent) : QObject(parent) {}

//#####################################################################################
// Functions

static QString formatTime(qint64 t, quint32 nanos = 0)
{
    if (t == 0) return QStringLiteral("-");

    QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(t), Qt::UTC);
    QString dtString = dt.toString(Qt::ISODate);

    if (nanos > 0)
        dtString += QString(".%1ns").arg(nanos, 9, 10, QChar('0'));

    return dtString;
}

static QString convertFileType(int type)
{
    switch (static_cast<TSK_FS_META_TYPE_ENUM>(type))
    {
        case TSK_FS_META_TYPE_REG:
            return QStringLiteral("Regular File");
        case TSK_FS_META_TYPE_DIR:
            return QStringLiteral("Directory");
        case TSK_FS_META_TYPE_FIFO:
            return QStringLiteral("FIFO/Pipe");
        case TSK_FS_META_TYPE_CHR:
            return QStringLiteral("Character Device");
        case TSK_FS_META_TYPE_BLK:
            return QStringLiteral("Block Device");
        case TSK_FS_META_TYPE_LNK:
            return QStringLiteral("Symbolic Link");
        case TSK_FS_META_TYPE_SOCK:
            return QStringLiteral("Socket");
        default:
            return QStringLiteral("N/A");
    }
}

static QString convertFlags(int flags)
{
    QStringList flagsString;

    if (flags & TSK_FS_META_FLAG_ALLOC)
        flagsString << QStringLiteral("Allocated");
    if (flags & TSK_FS_META_FLAG_UNALLOC)
        flagsString << QStringLiteral("Unallocated");
    if (flags & TSK_FS_META_FLAG_USED)
        flagsString << QStringLiteral("Used");
    if (flags & TSK_FS_META_FLAG_UNUSED)
        flagsString << QStringLiteral("Unused");
    if (flags & TSK_FS_META_FLAG_COMP)
        flagsString << QStringLiteral("Compressed");
    if (flags & TSK_FS_META_FLAG_ORPHAN)
        flagsString << QStringLiteral("Orphan");

    return flagsString.isEmpty() ? QStringLiteral("-") : flagsString.join(QStringLiteral(", "));
}

static QString formatPerm(int mode)
{
    if (mode == 0) return QStringLiteral("-");
    
    char charArray[16];
    charArray[0] = (mode & TSK_FS_META_MODE_IRUSR) ? 'r' : '-';
    charArray[1] = (mode & TSK_FS_META_MODE_IWUSR) ? 'w' : '-';
    charArray[2] = (mode & TSK_FS_META_MODE_IXUSR) ? 'x' : '-';
    charArray[3] = (mode & TSK_FS_META_MODE_IRGRP) ? 'r' : '-';
    charArray[4] = (mode & TSK_FS_META_MODE_IWGRP) ? 'w' : '-';
    charArray[5] = (mode & TSK_FS_META_MODE_IXGRP) ? 'x' : '-';
    charArray[6] = (mode & TSK_FS_META_MODE_IROTH) ? 'r' : '-';
    charArray[7] = (mode & TSK_FS_META_MODE_IWOTH) ? 'w' : '-';
    charArray[8] = (mode & TSK_FS_META_MODE_IXOTH) ? 'x' : '-';
    charArray[9] = '\0';

    return QString::fromLatin1(charArray) + QString(" (0%1)").arg(static_cast<int>(mode) & 0777, 3, 8, QChar('0'));
}


void MetadataViewer::fileSelected(const QString& diskImagePath, const QString& fsPath)
{
    QString errorString;
    tskFileReader tskReader;

    connect(&tskReader, &tskFileReader::fileOpened, this, [this](const TskFileMetadata& meta)
    {
        FileMetaFileMetadataDisplayStruct fileMetadata;

        fileMetadata.addr = QString::number(meta.addr);
        fileMetadata.type = convertFileType(meta.type);
        fileMetadata.flags = convertFlags(meta.flags);
        fileMetadata.mode = formatPerm(meta.mode);
        fileMetadata.size = QStringLiteral("%1 bytes").arg(meta.size);
        fileMetadata.uid = QString::number(meta.uid);
        fileMetadata.gid = QString::number(meta.gid);
        fileMetadata.nlink = QString::number(meta.nlink);
        fileMetadata.seq = meta.seq ? QString::number(meta.seq) : QStringLiteral("-");
        fileMetadata.mtime = formatTime(meta.mtime, meta.mtime_nano);
        fileMetadata.atime = formatTime(meta.atime, meta.atime_nano);
        fileMetadata.ctime = formatTime(meta.ctime, meta.ctime_nano);
        fileMetadata.crtime = formatTime(meta.crtime, meta.crtime_nano);
        fileMetadata.contentLen = QStringLiteral("%1 bytes").arg(meta.contentLen);
        fileMetadata.name = meta.hasName ? meta.name : QStringLiteral("-");
        fileMetadata.shrtName = meta.hasName ? meta.shrtName : QStringLiteral("-");
        fileMetadata.parAddr = meta.hasName ? QString::number(meta.parAddr) : QStringLiteral("-");

        emit metadataReady(fileMetadata);
    });

    tskReader.readFileMetadataFromImage(diskImagePath, fsPath, errorString);

    if (!errorString.isEmpty())
    {
        qWarning() << "[MetadataViewer]: Error Reading Metadata:" << errorString;

        FileMetaFileMetadataDisplayStruct fileMetadata;

        fileMetadata.addr = QStringLiteral("-"); 
        fileMetadata.type = QStringLiteral("-"); 
        fileMetadata.flags = QStringLiteral("-"); 
        fileMetadata.mode = QStringLiteral("-"); 
        fileMetadata.size = QStringLiteral("-"); 
        fileMetadata.uid = QStringLiteral("-"); 
        fileMetadata.gid = QStringLiteral("-"); 
        fileMetadata.nlink = QStringLiteral("-");
        fileMetadata.seq = QStringLiteral("-"); 
        fileMetadata.mtime = QStringLiteral("-"); 
        fileMetadata.atime = QStringLiteral("-"); 
        fileMetadata.ctime = QStringLiteral("-"); 
        fileMetadata.crtime = QStringLiteral("-"); 
        fileMetadata.contentLen = QStringLiteral("-");
        fileMetadata.name = QStringLiteral("-"); 
        fileMetadata.shrtName = QStringLiteral("-"); 
        fileMetadata.parAddr = QStringLiteral("-");

        emit metadataReady(fileMetadata);
    }
}
