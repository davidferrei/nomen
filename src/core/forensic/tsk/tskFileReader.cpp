#include "tskFileReader.h"

tskFileReader::tskFileReader(QObject* parent) : QObject(parent) {}

tskFileReader::~tskFileReader() {}


QByteArray tskFileReader::readFileContentFromImage(const QString& diskImagePath, const QString& fsPath, QString& errorStringOutput)
{
    const QByteArray imgBytes  = diskImagePath.toUtf8();
    const QByteArray pathBytes = fsPath.toUtf8();

    const int firstColon  = pathBytes.indexOf(':');
    const int secondColon = (firstColon >= 0) ? pathBytes.indexOf(':', firstColon + 1) : -1;

    if (firstColon < 0 || secondColon < 0) 
	{
        errorStringOutput = QString("[tskFileReader] Invalid FSPath Format:" + fsPath);
        qWarning() << errorStringOutput;

        return {};
    }

    bool isPartitionIdIntValid, isFsIndexIntValid;

    const int partitionId = pathBytes.left(firstColon).toInt(&isPartitionIdIntValid);
    const int fsIndex = pathBytes.mid(firstColon + 1, secondColon - firstColon - 1).toInt(&isFsIndexIntValid);

    const QByteArray actualPath = pathBytes.mid(secondColon + 1);

    if (!isPartitionIdIntValid || !isFsIndexIntValid) 
	{
        errorStringOutput = QString("[tskFileReader] Failed to Parse PartitionId/FSIndex from" + fsPath);
        qWarning() << errorStringOutput;

        return {};
    }

    TSK_IMG_INFO* img = tsk_img_open_sing(imgBytes.constData(), TSK_IMG_TYPE_DETECT, 0);
    if (!img) 
	{
        errorStringOutput = QString("[tskFileReader] Failed to Open Disk Image '%1': %2").arg(diskImagePath, QString::fromUtf8(tsk_error_get()));
        qWarning() << errorStringOutput;

        return {};
    }


    TSK_FS_INFO* fs = nullptr;

    TSK_VS_INFO* vs = tsk_vs_open(img, 0, TSK_VS_TYPE_DETECT);
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
			{
                fs = pfs;
            } 
			else 
			{
                tsk_fs_close(pfs);
            }
        }
        tsk_vs_close(vs);
    }
	else 
	{
		qWarning() << "[tskFileReader] No Volume System Detected, Attempting to Open Filesystem Directly";
        fs = tsk_fs_open_img(img, 0, TSK_FS_TYPE_DETECT);
    }

    if (!fs) 
	{
        errorStringOutput = QString("[tskFileReader] Failed to Open Filesystem for Partition %1 FS %2 in '%3': %4")
                                .arg(partitionId).arg(fsIndex).arg(diskImagePath, QString::fromUtf8(tsk_error_get()));
        qWarning() << errorStringOutput;
        tsk_img_close(img);
        return {};
    }

    TSK_FS_FILE* fs_file = tsk_fs_file_open(fs, nullptr, actualPath.constData());
    if (!fs_file) 
	{
        errorStringOutput = QString("[tskFileReader] Failed to Open File '%1': %2")
                                .arg(fsPath, QString::fromUtf8(tsk_error_get()));
        qWarning() << errorStringOutput;
        tsk_fs_close(fs);
        tsk_img_close(img);
        return {};
    }

    if (!fs_file->meta) 
	{
		//TODO: Handle Files with No Metadata
        errorStringOutput = QString("[tskFileReader] No Metadata for '%1'").arg(fsPath);
        qWarning() << errorStringOutput;
        tsk_fs_file_close(fs_file);
        tsk_fs_close(fs);
        tsk_img_close(img);
        return {};
    }

    const TSK_OFF_T fileSize = fs_file->meta->size;

	QByteArray result;
    result.resize(static_cast<qsizetype>(fileSize));

    constexpr size_t CHUNK = 65536; // 64 KB
    TSK_OFF_T offset = 0;

	while (offset < fileSize) 
	{
        const size_t toRead = static_cast<size_t>(std::min(static_cast<TSK_OFF_T>(CHUNK), fileSize - offset));

        const ssize_t n = tsk_fs_file_read(
            fs_file, offset,
            result.data() + offset, toRead,
            TSK_FS_FILE_READ_FLAG_NONE);

        if (n < 0) 
		{
            errorStringOutput = QString("[tskFileReader] Read Error at Offset %1 in '%2': %3")
                                    .arg(offset).arg(fsPath, QString::fromUtf8(tsk_error_get()));
            qWarning() << errorStringOutput;

            tsk_fs_file_close(fs_file);
            tsk_fs_close(fs);
            tsk_img_close(img);

            return {};
        }

        if (n == 0) break;

        offset += n;
    }

    result.resize(static_cast<qsizetype>(offset));

    tsk_fs_file_close(fs_file);
    tsk_fs_close(fs);
    tsk_img_close(img);

    return result;
}
