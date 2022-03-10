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
#include <string>
#include <utility>
#include <vector>

#include "meta_storage_specific.h"
#include "src/metafs/mvm/volume/meta_volume.h"
#include "meta_volume_type.h"
#include "os_header.h"

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
    MetaVolumeContainer(void);
    virtual ~MetaVolumeContainer(void);

    // MetaVolume* vol is for test
    virtual void InitContext(MetaVolumeType volumeType, int arrayId,
                MetaLpnType maxVolPageNum, MetaStorageSubsystem* metaStorage,
                MetaVolume* vol = nullptr);

    virtual bool CreateVolume(MetaVolumeType volumeType);
    virtual bool OpenAllVolumes(bool isNPOR);
    virtual bool CloseAllVolumes(bool& resetContext /*output*/);
    virtual bool IsGivenVolumeExist(MetaVolumeType volType);

    virtual bool TrimData(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);

    virtual bool CreateFile(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    virtual bool DeleteFile(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);

    virtual size_t GetAvailableSpace(MetaVolumeType volType);
    virtual bool CheckFileInActive(MetaVolumeType volType, FileDescriptorType fd);
    virtual POS_EVENT_ID AddFileInActiveList(MetaVolumeType volType, FileDescriptorType fd);
    virtual bool IsGivenFileCreated(std::string& fileName);
    virtual void RemoveFileFromActiveList(MetaVolumeType volType, FileDescriptorType fd);
    virtual FileSizeType GetFileSize(MetaVolumeType volType, FileDescriptorType fd);
    virtual FileSizeType GetDataChunkSize(MetaVolumeType volType, FileDescriptorType fd);
    virtual MetaLpnType GetFileBaseLpn(MetaVolumeType volType, FileDescriptorType fd);
    virtual MetaLpnType GetMaxLpn(MetaVolumeType volType);
    virtual FileDescriptorType LookupFileDescByName(std::string& fileName);
    virtual MetaFileInode& GetInode(FileDescriptorType fd, MetaVolumeType volumeType);
    virtual void GetInodeList(std::vector<MetaFileInfoDumpCxt>*& fileInfoList, MetaVolumeType volumeType);
    virtual bool CopyInodeToInodeInfo(FileDescriptorType fd,
                    MetaVolumeType volumeType, MetaFileInodeInfo* inodeInfo /* output */);

    virtual POS_EVENT_ID DetermineVolumeToCreateFile(FileSizeType fileByteSize,
                    MetaFilePropertySet& prop, MetaVolumeType volumeType);
    virtual POS_EVENT_ID LookupMetaVolumeType(FileDescriptorType fd,
                    MetaVolumeType volumeType);
    virtual POS_EVENT_ID LookupMetaVolumeType(std::string& fileName,
                    MetaVolumeType volumeType);
    virtual MetaLpnType GetTheLastValidLpn(MetaVolumeType volumeType);

private:
    MetaVolume* _InitVolume(MetaVolumeType volType, int arrayId,
                    MetaLpnType maxLpnNum, MetaStorageSubsystem* metaStorage,
                    MetaVolume* vol);
    void _RegisterVolumeInstance(MetaVolumeType volType, MetaVolume* metaVol);
    void _CleanUp(void);

    bool nvramMetaVolAvailable;
    bool allVolumesOpened;
    int arrayId;

    std::map<MetaVolumeType, MetaVolume*> volumeContainer;

    void _SetBackupInfo(MetaVolume* volume, MetaLpnType* info);
};
} // namespace pos
