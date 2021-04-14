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

#pragma once

#include <map>
#include <string>
#include <utility>

#include "meta_storage_specific.h"
#include "file_descriptor_manager.h"
#include "meta_volume.h"
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

class MetaVolumeContainer
{
public:
    MetaVolumeContainer(void);
    ~MetaVolumeContainer(void);

    bool OpenAllVolumes(bool isNPOR);
    bool CloseAllVolumes(bool& resetContext /*output*/);
    void RegisterVolumeInstance(MetaVolumeType volType, MetaVolume* metaVol);
    bool IsNvRamVolumeAvailable(void);
    bool IsGivenVolumeExist(MetaVolumeType volumeType);
    bool IsGivenFileCreated(std::string fileName);
#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(void);
#endif
    MetaVolume& GetMetaVolume(MetaVolumeType volumeType);
    std::pair<MetaVolumeType, POS_EVENT_ID> DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop);
    std::pair<MetaVolumeType, POS_EVENT_ID> LookupMetaVolumeType(FileDescriptorType fd);
    std::pair<MetaVolumeType, POS_EVENT_ID> LookupMetaVolumeType(std::string& fileName);
    void UpdateVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd, MetaVolumeType volumeType);
    void RemoveVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd);
    void AddAllFDsInFreeFDMap(void);
    void BuildFreeFDMap(void);
    void BuildFDLookup(void);
    void ResetFileDescriptorManager(void);
    void InsertFileDescLookupHash(std::string& fileName, FileDescriptorType fd);
    void EraseFileDescLookupHash(std::string& fileName);
    FileDescriptorType FindFDByName(std::string fileName);
    FileDescriptorType AllocFileDescriptor(std::string fileName);
    void FreeFileDescriptor(FileDescriptorType fd);

    inline bool
    GetVolOpenFlag(void)
    {
        return allVolumesOpened;
    }

private:
    bool _CheckOkayToStore(MetaVolumeType volumeType, FileSizeType fileByteSize, MetaFilePropertySet& prop);
    void _BuildFD2VolumeTypeMap(MetaVolume* volume);
    void _BuildFileKey2VolumeTypeMap(MetaVolume* volume);
    void _CleanUp(void);

    bool nvramMetaVolAvailable;
    bool allVolumesOpened;
    std::map<MetaVolumeType, MetaVolume*> volumeContainer;
    std::unordered_map<FileDescriptorType, MetaVolumeType> fd2VolTypehMap;
    std::unordered_map<StringHashType, MetaVolumeType> fileKey2VolTypeMap;
    FileDescriptorManager fdManager;

    void _SetBackupInfo(MetaVolume* volume, MetaLpnType* info);
};
} // namespace pos
