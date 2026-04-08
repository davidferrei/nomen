#include "tskFileReader.h"


tskFileReader::tskFileReader(QObject* parent) : QObject(parent) {}

tskFileReader::~tskFileReader() {}


static bool openTskFile(const QString& diskImagePath, const QString& fsPath, TSK_IMG_INFO*& TSK_img, TSK_FS_INFO*& TSK_fs, TSK_FS_FILE*& TSK_file, QString& errorStringOutput)
{
    const QByteArray imgBytes  = diskImagePath.toUtf8();
    const QByteArray pathBytes = fsPath.toUtf8();

    const int firstColon  = pathBytes.indexOf(':');
    const int secondColon = (firstColon >= 0) ? pathBytes.indexOf(':', firstColon + 1) : -1;

    if (firstColon < 0 || secondColon < 0)
    {
        errorStringOutput = QString("[tskFileReader] Invalid FSPath Format:" + fsPath);
        qWarning() << errorStringOutput;
        return false;
    }

    bool isPartitionIdIntValid, isFsIndexIntValid;
    const int partitionId = pathBytes.left(firstColon).toInt(&isPartitionIdIntValid);
    const int fsIndex     = pathBytes.mid(firstColon + 1, secondColon - firstColon - 1).toInt(&isFsIndexIntValid);
    const QByteArray actualPath = pathBytes.mid(secondColon + 1);

    if (!isPartitionIdIntValid || !isFsIndexIntValid)
    {
        errorStringOutput = QString("[tskFileReader] Failed to Parse PartitionId/FSIndex from" + fsPath);
        qWarning() << errorStringOutput;
        return false;
    }

    TSK_img = tsk_img_open_sing(imgBytes.constData(), TSK_IMG_TYPE_DETECT, 0);
    if (!TSK_img)
    {
        errorStringOutput = QString("[tskFileReader] Failed to Open Disk Image '%1': %2").arg(diskImagePath, QString::fromUtf8(tsk_error_get()));
        qWarning() << errorStringOutput;
        return false;
    }

    TSK_fs = nullptr;
    TSK_VS_INFO* vs = tsk_vs_open(TSK_img, 0, TSK_VS_TYPE_DETECT);
    if (vs)
    {
        std::unordered_map<int, int> fsIdxMap;
        for (TSK_PNUM_T i = 0; i < vs->part_count; i++)
        {
            const TSK_VS_PART_INFO* part = tsk_vs_part_get(vs, i);
            if (!part) continue;

            TSK_FS_INFO* pfs = tsk_fs_open_vol(part, TSK_FS_TYPE_DETECT);
            if (!pfs) continue;

            const int pid  = static_cast<int>(i);
            const int fidx = fsIdxMap[pid]++;

            if (pid == partitionId && fidx == fsIndex)
                TSK_fs = pfs;
            else
                tsk_fs_close(pfs);
        }
        tsk_vs_close(vs);
    }
    else
    {
        qWarning() << "[tskFileReader] No Volume System Detected, Attempting to Open Filesystem Directly";
        TSK_fs = tsk_fs_open_img(TSK_img, 0, TSK_FS_TYPE_DETECT);
    }

    if (!TSK_fs)
    {
        errorStringOutput = QString("[tskFileReader] Failed to Open Filesystem for Partition %1 FS %2 in '%3': %4")
                                .arg(partitionId).arg(fsIndex).arg(diskImagePath, QString::fromUtf8(tsk_error_get()));
        qWarning() << errorStringOutput;
        tsk_img_close(TSK_img);
        return false;
    }

    TSK_file = tsk_fs_file_open(TSK_fs, nullptr, actualPath.constData());
    if (!TSK_file)
    {
        errorStringOutput = QString("[tskFileReader] Failed to Open File '%1': %2").arg(fsPath, QString::fromUtf8(tsk_error_get()));
        qWarning() << errorStringOutput;

        tsk_fs_close(TSK_fs);
        tsk_img_close(TSK_img);

        return false;
    }

    if (!TSK_file->meta)
    {
        //TODO: Handle Files with No Metadata
        errorStringOutput = QString("[tskFileReader] No Metadata for '%1'").arg(fsPath);
        qWarning() << errorStringOutput;

        tsk_fs_file_close(TSK_file);
        tsk_fs_close(TSK_fs);
        tsk_img_close(TSK_img);

        return false;
    }

    TSK_img  = TSK_img;
    TSK_fs   = TSK_fs;
    TSK_file = TSK_file;

    return true;
}


static QByteArray readChunk(TSK_FS_FILE* TSK_file, TSK_OFF_T offset, TSK_OFF_T chunckLength, const QString& fsPath, QString& errorStringOutput)
{
    QByteArray resultBArray;
    resultBArray.resize(static_cast<qsizetype>(chunckLength));

    constexpr size_t CHUNK = 65536; // 64 KB
    TSK_OFF_T cursor = 0;

    while (cursor < chunckLength)
    {
        const size_t toRead = static_cast<size_t>(std::min(static_cast<TSK_OFF_T>(CHUNK), chunckLength - cursor));

        const ssize_t n = tsk_fs_file_read(
            TSK_file, offset + cursor,
            resultBArray.data() + cursor, toRead,
            TSK_FS_FILE_READ_FLAG_NONE);

        if (n < 0)
        {
            errorStringOutput = QString("[tskFileReader] Read Error at Offset %1 in '%2': %3").arg(offset + cursor).arg(fsPath, QString::fromUtf8(tsk_error_get()));
            qWarning() << errorStringOutput;
            return {};
        }

        if (n == 0) break;

        cursor += n;
    }

    resultBArray.resize(static_cast<qsizetype>(cursor));
    return resultBArray;
}


QByteArray tskFileReader::readFileContentFromImage(const QString& diskImagePath, const QString& fsPath, qint64 offset, qint64 length, qint64& fileSize, QString& errorStringOutput)
{
    TSK_IMG_INFO* TSK_img   = nullptr;
    TSK_FS_INFO*  TSK_fs    = nullptr;
    TSK_FS_FILE*  TSK_file  = nullptr;

    if (!openTskFile(diskImagePath, fsPath, TSK_img, TSK_fs, TSK_file, errorStringOutput))
    {
        errorStringOutput = QString("[tskFileReader] Failed to Open TSK File for Reading");
        qWarning() << errorStringOutput;
        return {};
    }

    fileSize = static_cast<qint64>(TSK_file->meta->size);

    const TSK_OFF_T clampedOffset = static_cast<TSK_OFF_T>(std::min(offset, fileSize));
    const TSK_OFF_T chunckLength = static_cast<TSK_OFF_T>(std::min(length, fileSize - clampedOffset));

    QByteArray result = readChunk(TSK_file, clampedOffset, chunckLength, fsPath, errorStringOutput);

    tsk_fs_file_close(TSK_file);
    tsk_fs_close(TSK_fs);
    tsk_img_close(TSK_img);

    return result;
}
