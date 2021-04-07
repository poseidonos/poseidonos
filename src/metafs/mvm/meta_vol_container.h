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
#include "meta_vol_base.h"
#include "meta_vol_type.h"
#include "os_header.h"

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

class MetaVolContainerClass
{
public:
    MetaVolContainerClass(void);
    ~MetaVolContainerClass(void);

    bool OpenAllVolumes(bool isNPOR);
    bool CloseAllVolumes(bool& resetContext /*output*/);
    void RegisterVolumeInstance(MetaVolumeType volType, MetaVolumeClass* metaVol);
    void SetNvRamVolumeAvailable(void);
    bool IsNvRamVolumeAvailable(void);
    bool IsGivenVolumeExist(MetaVolumeType volumeType);
#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(void);
#endif
    MetaVolumeClass& GetMetaVolume(MetaVolumeType volumeType);
    std::pair<MetaVolumeType, IBOF_EVENT_ID> DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop);
    std::pair<MetaVolumeType, IBOF_EVENT_ID> LookupMetaVolumeType(FileFDType fd);
    std::pair<MetaVolumeType, IBOF_EVENT_ID> LookupMetaVolumeType(std::string& fileName);
    void UpdateVolumeLookupInfo(StringHashType fileHashKey, FileFDType fd, MetaVolumeType volumeType);
    void RemoveVolumeLookupInfo(StringHashType fileHashKey, FileFDType fd);
    void BuildFreeFDMap(std::map<FileFDType, FileFDType>& dstFreeFDMap);
    void BuildFDLookup(std::unordered_map<StringHashType, FileFDType>& fileKeyLookupMap);
    inline bool
    GetVolOpenFlag(void)
    {
        return allVolumesOpened;
    }

private:
    bool _CheckOkayToStore(MetaVolumeType volumeType, FileSizeType fileByteSize, MetaFilePropertySet& prop);
    void _BuildFD2VolumeTypeMap(MetaVolumeClass* volume);
    void _BuildFileKey2VolumeTypeMap(MetaVolumeClass* volume);
    void _CleanUp(void);

    bool nvramMetaVolAvailable;
    bool allVolumesOpened;
    std::map<MetaVolumeType, MetaVolumeClass*> volumeContainer;
    std::unordered_map<FileFDType, MetaVolumeType> fd2VolTypehMap;
    std::unordered_map<StringHashType, MetaVolumeType> fileKey2VolTypeMap;

    void _SetBackupInfo(MetaVolumeClass* volume, MetaLpnType* info);
};
