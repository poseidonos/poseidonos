/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <string>

#include "rocksdb/db.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/meta_file_intf/meta_file_intf.h"
#include "src/metafs/include/metafs_aiocb_cxt.h"
#include "src/metafs/metafs.h"
#include "src/metafs/mvm/volume/file_descriptor_allocator.h"

namespace pos
{
class MetaFsConfigManager;

class RocksDBMetaFsIntf : public MetaFileIntf
{
public:
    explicit RocksDBMetaFsIntf(const std::string fileName, const int arrayId,
        const MetaFileType fileType = MetaFileType::Map,
        const MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    // only for test
    explicit RocksDBMetaFsIntf(const std::string fileName, const int arrayId, MetaFs* metafs,
        MetaFsConfigManager* configManager,
        const MetaFileType fileType = MetaFileType::Map,
        const MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual ~RocksDBMetaFsIntf(void) override;

    virtual int Create(uint64_t fileSize) override;
    virtual bool DoesFileExist(void) override;
    virtual int Delete(void) override;
    virtual uint64_t GetFileSize(void) override;

    virtual int AsyncIO(AsyncMetaFileIoCtx* ctx) override;
    virtual int CheckIoDoneStatus(void* data) override;
    virtual int ReleaseAsyncIoContext(void* data);

    virtual int Open(void) override;
    virtual int Close(void) override;

    virtual int CreateDirectory(std::string pathName);
    virtual int DeleteDirectory(void);
    virtual int SetRocksMeta(std::string pathName);
    virtual int DeleteRocksMeta(void);

protected:
    virtual int _Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer) override;
    virtual int _Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer) override;
    MetaFs* metaFs;
    uint32_t blksPerStripe;
    MetaLpnType baseLpn;
    const bool BYTE_ACCESS_ENABLED;
    std::string pathName;

private:
    virtual int _AsyncIOWrite(AsyncMetaFileIoCtx* ctx);
    virtual int _AsyncIORead(AsyncMetaFileIoCtx* ctx);
    rocksdb::DB* rocksMeta;
    FileDescriptorAllocator* fileDescriptorAllocator;
    inline std::string
    _MakeRocksDbKey(FileDescriptorType fd, FileOffsetType offset)
    {
        // RocksDB key example
        // if fd :1 , offset : 2 -> key == "0000000001_00000000000000000002"
        int fdMaxLen = 10;
        int offsetMaxLen = 20;
        int fdZeroSize = fdMaxLen - std::to_string(fd).size();
        int offsetZeroSize = offsetMaxLen - std::to_string(offset).size();

        assert(fdZeroSize >= 0);
        assert(offsetZeroSize >= 0);

        std::string appendFdZero(fdZeroSize, '0');
        std::string appendOffsetZero(offsetZeroSize, '0');
        std::string key = appendFdZero + std::to_string(fd) + "_" + appendOffsetZero + std::to_string(offset);
        return key;
    }
    inline std::string
    _MakeRocksDbInodeKey(std::string fileName)
    {
        std::string key = "inode_" + fileName;
        return key;
    }

    inline std::string
    _MakeRocksDbInode(FileDescriptorType fd, uint64_t fileSize)
    {
        int fdMaxLen = 10;
        int fdZeroSize = fdMaxLen - std::to_string(fd).size();
        std::string appendFdZero(fdZeroSize, '0');
        std::string inode = appendFdZero + std::to_string(fd) + "_" + std::to_string(fileSize);
        return inode;
    }

    inline pair<FileDescriptorType, uint64_t>
    _GetValueFromInode(std::string inode)
    {
        int stringLength = inode.size();
        int pos = inode.find('_');
        FileDescriptorType fd = atoi(inode.substr(0, pos).c_str());
        uint64_t size = atoll(inode.substr(pos + 1, stringLength).c_str());
        return {fd, size};
    }

    inline uint64_t
    _GetOffsetFromKey(std::string key)
    {
        int stringLength = key.size();
        int pos = key.find('_');
        uint64_t offset = atoll(key.substr(pos + 1, stringLength).c_str());
        return offset;
    }
};

} // namespace pos
