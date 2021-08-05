/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

/* 
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta File Management API
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "src/lib/bitmap.h"
#include "src/metafs/storage/mss.h"
#include "src/metafs/util/metafs_spinlock.h"
#include "src/metafs/mvm/meta_volume_manager.h"
#include "src/metafs/include/meta_file_context.h"

namespace pos
{
class MetaFsFileControlApi
{
public:
    MetaFsFileControlApi(void);
    explicit MetaFsFileControlApi(int arrayId);
    virtual ~MetaFsFileControlApi(void);

    virtual POS_EVENT_ID Create(std::string& fileName, uint64_t fileByteSize,
        MetaFilePropertySet prop = MetaFilePropertySet());
    virtual POS_EVENT_ID Delete(std::string& fileName);
    virtual POS_EVENT_ID Open(std::string& fileName, int& fd);
    virtual POS_EVENT_ID Close(FileDescriptorType fd);
    virtual POS_EVENT_ID CheckFileExist(std::string& fileName);
    virtual size_t GetFileSize(int fd);
    virtual size_t GetAlignedFileIOSize(int fd);
    virtual size_t EstimateAlignedFileIOSize(MetaFilePropertySet& prop);
    virtual size_t GetTheBiggestExtentSize(MetaFilePropertySet& prop);
    virtual size_t GetMaxMetaLpn(MetaVolumeType type);
    virtual void SetStatus(bool isNormal);
    MetaFileContext* GetFileInfo(FileDescriptorType fd);

    // for wbt commands
    virtual std::vector<MetaFileInfoDumpCxt> Wbt_GetMetaFileList(void);
    virtual FileSizeType Wbt_GetMaxFileSizeLimit(void);
    virtual MetaFileInodeInfo* Wbt_GetMetaFileInode(std::string& fileName);

    virtual void SetMss(MetaStorageSubsystem* metaStorage);
    virtual void InitVolume(MetaVolumeType volType, int arrayId, MetaLpnType maxVolPageNum);
    virtual bool CreateVolume(MetaVolumeType volType);
    virtual bool OpenVolume(bool isNPOR);
    virtual bool CloseVolume(bool& isNPOR);

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    virtual bool Compaction(bool isNPOR);
#endif

private:
    MetaFileInodeInfo* _GetFileInode(std::string& fileName);
    void _AddFileContext(std::string& fileName, FileDescriptorType fd);
    void _RemoveFileContext(FileDescriptorType fd);

    int arrayId = INT32_MAX;
    bool isNormal = false;
    MetaVolumeManager* volMgr = nullptr;

    BitMap* bitmap = nullptr;
    // (fd, array)
    std::unordered_map<FileDescriptorType, std::string> nameMapByfd;
    // (array, file index)
    std::unordered_map<std::string, uint32_t> idxMapByName;
    MetaFileContext cxtList[MetaFsConfig::MAX_VOLUME_CNT];
    MetaFsSpinLock iLock;
};
} // namespace pos
