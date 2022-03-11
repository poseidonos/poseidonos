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

#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "meta_storage_specific.h"
#include "meta_volume_type.h"
#include "os_header.h"
#include "src/metafs/mvm/volume/meta_volume.h"

namespace pos
{
enum class BackupInfo
{
    First = 0,
    BaseLpn = First,
    CatalogSize,
    InodeHdrSize,
    InodeTableSize,
    Last = InodeTableSize,

    Max,
    Invalid = Max,
};

using VolumeAndResult = std::pair<MetaVolumeType, POS_EVENT_ID>;

class MetaVolumeContainer
{
public:
    MetaVolumeContainer(const int arrayId);
    virtual ~MetaVolumeContainer(void);

    // std::shared_ptr<MetaVolume> vol is for test
    virtual void InitContext(const MetaVolumeType volumeType, const int arrayId,
        const MetaLpnType maxVolPageNum, MetaStorageSubsystem* metaStorage,
        std::shared_ptr<MetaVolume> vol = nullptr);

    virtual bool CreateVolume(const MetaVolumeType volumeType);
    virtual bool OpenAllVolumes(bool isNPOR);
    virtual bool CloseAllVolumes(bool& resetContext /*output*/);
    virtual bool IsGivenVolumeExist(const MetaVolumeType volumeType);

    virtual bool TrimData(const MetaVolumeType volumeType, MetaFsFileControlRequest& reqMsg);

    virtual bool CreateFile(const MetaVolumeType volumeType, MetaFsFileControlRequest& reqMsg);
    virtual bool DeleteFile(const MetaVolumeType volumeType, MetaFsFileControlRequest& reqMsg);

    virtual size_t GetAvailableSpace(const MetaVolumeType volumeType);
    virtual bool CheckFileInActive(const MetaVolumeType volumeType, const FileDescriptorType fd);
    virtual POS_EVENT_ID AddFileInActiveList(const MetaVolumeType volumeType, const FileDescriptorType fd);
    virtual bool IsGivenFileCreated(std::string& fileName);
    virtual void RemoveFileFromActiveList(const MetaVolumeType volumeType, const FileDescriptorType fd);
    virtual FileSizeType GetFileSize(const MetaVolumeType volumeType, const FileDescriptorType fd);
    virtual FileSizeType GetDataChunkSize(const MetaVolumeType volumeType, const FileDescriptorType fd);
    virtual MetaLpnType GetFileBaseLpn(const MetaVolumeType volumeType, const FileDescriptorType fd);
    virtual MetaLpnType GetMaxLpn(const MetaVolumeType volumeType);
    virtual FileDescriptorType LookupFileDescByName(std::string& fileName);
    virtual MetaFileInode& GetInode(const FileDescriptorType fd, const MetaVolumeType volumeType);
    virtual void GetInodeList(std::vector<MetaFileInfoDumpCxt>*& fileInfoList, const MetaVolumeType volumeType);
    virtual bool CopyInodeToInodeInfo(FileDescriptorType fd,
        const MetaVolumeType volumeType, MetaFileInodeInfo* inodeInfo /* output */);

    virtual POS_EVENT_ID DetermineVolumeToCreateFile(FileSizeType fileByteSize,
        MetaFilePropertySet& prop, const MetaVolumeType volumeType);
    virtual POS_EVENT_ID LookupMetaVolumeType(FileDescriptorType fd,
        const MetaVolumeType volumeType);
    virtual POS_EVENT_ID LookupMetaVolumeType(std::string& fileName,
        const MetaVolumeType volumeType);
    virtual MetaLpnType GetTheLastValidLpn(const MetaVolumeType volumeType);

private:
    std::shared_ptr<MetaVolume> _CreateVolume(const MetaVolumeType volumeType,
        const int arrayId, const MetaLpnType maxLpnNum,
        MetaStorageSubsystem* metaStorage);
    void _CleanUp(void);

    bool nvramMetaVolAvailable;
    bool allVolumesOpened;
    int arrayId;

    std::map<MetaVolumeType, std::shared_ptr<MetaVolume>> volumeContainer;

    void _SetBackupInfo(std::shared_ptr<MetaVolume> volume, MetaLpnType* info);
};
} // namespace pos
