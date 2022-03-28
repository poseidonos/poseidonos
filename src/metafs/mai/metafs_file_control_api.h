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

/* 
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta File Management API
*/

#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/lib/bitmap.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/metafs/include/meta_file_context.h"
#include "src/metafs/mai/metafs_management_api.h"
#include "src/metafs/mvm/meta_volume_manager.h"
#include "src/metafs/storage/mss.h"
#include "src/metafs/util/metafs_spinlock.h"

namespace pos
{
class MetaFsFileControlApi
{
public:
    MetaFsFileControlApi(void);
    explicit MetaFsFileControlApi(const int arrayId, MetaStorageSubsystem* storage,
        MetaFsManagementApi* mgmt, MetaVolumeManager* volMgr = nullptr);
    virtual ~MetaFsFileControlApi(void);

    virtual POS_EVENT_ID Create(std::string& fileName, uint64_t fileByteSize,
        MetaFilePropertySet prop = MetaFilePropertySet(),
        MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual POS_EVENT_ID Delete(std::string& fileName,
        MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual POS_EVENT_ID Open(std::string& fileName, int& fd,
        MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual POS_EVENT_ID Close(FileDescriptorType fd,
        MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual POS_EVENT_ID CheckFileExist(std::string& fileName,
        MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual size_t GetFileSize(int fd,
        MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual size_t GetAlignedFileIOSize(int fd,
        MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual size_t EstimateAlignedFileIOSize(MetaFilePropertySet& prop,
        MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual size_t GetAvailableSpace(MetaFilePropertySet& prop,
        MetaVolumeType volumeType = MetaVolumeType::SsdVolume);
    virtual size_t GetMaxMetaLpn(MetaVolumeType type);
    virtual void SetStatus(bool isNormal);
    virtual MetaFileContext* GetFileInfo(FileDescriptorType fd, MetaVolumeType type);
    virtual MetaLpnType GetTheLastValidLpn(MetaVolumeType type)
    {
        return volMgr->GetTheLastValidLpn(type);
    }

    // for wbt commands
    virtual std::vector<MetaFileInfoDumpCxt> Wbt_GetMetaFileList(MetaVolumeType type);
    virtual MetaFileInodeInfo* Wbt_GetMetaFileInode(std::string& fileName,
        MetaVolumeType type);

    virtual void InitVolume(MetaVolumeType volType, int arrayId, MetaLpnType maxVolPageNum);
    virtual bool CreateVolume(MetaVolumeType volType);
    virtual bool OpenVolume(bool isNPOR);
    virtual bool CloseVolume(bool& isNPOR);

protected:
    MetaFileInodeInfo* _GetFileInode(std::string& fileName,
        MetaVolumeType type);
    void _AddFileContext(std::string& fileName, FileDescriptorType fd,
        MetaVolumeType type);
    void _RemoveFileContext(FileDescriptorType fd, MetaVolumeType type);

private:
    int arrayId = INT32_MAX;
    bool isNormal = false;
    MetaStorageSubsystem* storage;
    MetaFsManagementApi* mgmt;
    MetaVolumeManager* volMgr = nullptr;

    BitMap* bitmap = nullptr;
    // (pair<MetaVolumeType, fd>, fileName)
    std::unordered_map<std::pair<MetaVolumeType, FileDescriptorType>, std::string, PairHash> nameMapByfd;
    // (pair<MetaVolumeType, fileName>, file index)
    std::unordered_map<std::pair<MetaVolumeType, std::string>, uint32_t, PairHash> idxMapByName;
    MetaFileContext cxtList[MetaFsConfig::MAX_VOLUME_CNT];
    MetaFsSpinLock iLock;
};
} // namespace pos
