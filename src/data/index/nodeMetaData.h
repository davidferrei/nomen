#ifndef NODE_METADATA_H
#define NODE_METADATA_H

#include <cstdint>

struct NodeMetadata {
    //TODO: Add Support for More Metadata (atime, crtime, permissions, owner, group, etc.)
    uint64_t inode;
    uint64_t size;
    uint64_t ctime;
    uint64_t mtime;
    uint8_t type;
    uint8_t is_dir;
    uint8_t status;
        // Status: 0=Allocated, 1=Unallocated (meta intact), 2=Deleted (meta lost), 3=Orphan, 4=Raw (no filesystem)

};

#endif // NODE_METADATA_H
