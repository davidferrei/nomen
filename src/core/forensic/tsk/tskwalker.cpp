#include "tskwalker.h"

#include <QDebug>
#include <tsk/libtsk.h>

static TSK_WALK_RET_ENUM file_callback(
    TSK_FS_FILE *fs_file,
    const char *path,
    void * /*ptr*/)
{
    if (!fs_file || !fs_file->name || !fs_file->name->name)
        return TSK_WALK_CONT;

    QString name = QString::fromUtf8(fs_file->name->name);
    QString fullPath = QString::fromUtf8(path) + "/" + name;

    qDebug() << fullPath;

    return TSK_WALK_CONT;
}

bool TskWalker::walkImage(const QString &imagePath)
{
    TSK_IMG_INFO *img =
        tsk_img_open_sing(imagePath.toUtf8().data(),
                          TSK_IMG_TYPE_DETECT,
                          0);

    if (!img) {
        qWarning() << "TSK: Failed to Open Image";
        return false;
    }

    TSK_FS_INFO *fs = nullptr;
    TSK_VS_INFO *vs = tsk_vs_open(img, 0, TSK_VS_TYPE_DETECT);

    if (vs) {
        for (int i = 0; i < vs->part_count; i++) {
            const TSK_VS_PART_INFO *part = tsk_vs_part_get(vs, i);
            if (!part) continue;

            fs = tsk_fs_open_vol(part, TSK_FS_TYPE_DETECT);
            if (fs) break;
        }
        tsk_vs_close(vs);
    } else {
        fs = tsk_fs_open_img(img, 0, TSK_FS_TYPE_DETECT);
    }

    if (!fs) {
        qWarning() << "TSK: Failed to Open Filesystem";
        tsk_img_close(img);
        return false;
    }

    tsk_fs_dir_walk(
        fs,
        fs->root_inum,
        TSK_FS_DIR_WALK_FLAG_RECURSE,
        file_callback,
        nullptr);

    tsk_fs_close(fs);
    tsk_img_close(img);

    return true;
}
