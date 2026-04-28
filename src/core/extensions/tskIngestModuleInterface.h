#ifndef TSKINGESTMODULEINTERFACE_H
#define TSKINGESTMODULEINTERFACE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVector>
#include <tsk/libtsk.h>

#include "core/extensions/ingestModuleInterface.h"

struct FileContext
{
    TSK_FS_FILE* fs_file = nullptr;
    const char* parent_path = nullptr;
    QString filename; 
    QString filePath;
    TSK_FS_META* tsk_metadata = nullptr;
    bool hasMetadata = false;
    int partitionId = 0;
    int fsIndex = 0;

    ssize_t readFileContent(uint64_t offset, char* buf, size_t len) const
    {
        if (!fs_file) return -1;

        return tsk_fs_file_read(fs_file, offset, buf, len, TSK_FS_FILE_READ_FLAG_NONE);
    }
};

class tskIngestModuleInterface : public AbstractIngestModule
{
public:
    explicit tskIngestModuleInterface(const QJsonObject &metadata = QJsonObject(), QObject *parent = nullptr) : AbstractIngestModule(metadata, parent) {}
    ~tskIngestModuleInterface() override = default;

    void process() override {}

    bool runWalk(const QString &imagePath);

    virtual void processFile(const FileContext &fc) = 0;

    virtual void finishWalk() {}

protected:
    static TSK_WALK_RET_ENUM file_callback(TSK_FS_FILE* fs_file, const char* path, void* ptr);

    int currentPartitionId = 0;
    int currentFsIndex = 0;
};

#endif // TSKINGESTMODULEINTERFACE_H
