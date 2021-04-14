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
 * Meta Volume Container Map
*/
#pragma once

#include <string>
#include <atomic>
#include <unordered_map>
#include <utility>
#include "meta_volume_container.h"
#include "file_descriptor_manager.h"
#include "nvram_meta_volume.h"
#include "ssd_meta_volume.h"
#include "src/lib/bitmap.h"

namespace pos
{
// A Group of Meta Volume Containers
class MetaVolumeContainerMap
{
public:
    MetaVolumeContainerMap(void);
    ~MetaVolumeContainerMap(void);

    void RegisterVolumeInstance(MetaVolumeType volumeType,
                    std::string arrayName, MetaVolume* volume);
    MetaVolume& GetMetaVolume(MetaVolumeType volType, std::string arrayName);
    bool IsGivenVolumeExist(MetaVolumeType volType, std::string arrayName);
    bool IsGivenFileCreated(std::string fileName, std::string arrayName);
    bool GetVolOpenFlag(std::string arrayName);
    MetaLpnType GetMaxMetaLpn(MetaVolumeType volType, std::string arrayName);

    bool OpenAllVolumes(bool isNPOR, std::string arrayName);
    bool CloseAllVolumes(bool& resetContext, std::string arrayName);

    std::pair<MetaVolumeType, POS_EVENT_ID> IsPossibleToCreateFile(
                        FileSizeType fileByteSize, MetaFilePropertySet& prop,
                        std::string arrayName);
    std::pair<MetaVolumeType, POS_EVENT_ID> FindMetaVolumeType(FileDescriptorType fd,
                        std::string arrayName);
    std::pair<MetaVolumeType, POS_EVENT_ID> FindMetaVolumeType(std::string& fileName,
                        std::string arrayName);

    void UpdateVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd,
                        std::string arrayName, MetaVolumeType volumeType);
    void RemoveVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd,
                        std::string arrayName);

    FileDescriptorType FindFDByName(std::string fileName, std::string arrayName);
    FileDescriptorType AllocFileDescriptor(std::string fileName, std::string arrayName);
    void FreeFileDescriptor(std::string& fileName, FileDescriptorType fd, std::string& arrayName);
    void AddAllFDsInFreeFDMap(std::string arrayName);
    void BuildFreeFDMap(std::string arrayName);
    void BuildFDLookup(std::string arrayName);

    void InsertFileDescLookupHash(std::string& fileName, FileDescriptorType fd,
                        std::string& arrayName);
    void RemoveFileDescLookupHash(std::string& fileName, std::string& arrayName);

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(std::string arrayName);
#endif

private:
    MetaVolumeContainer* _GetMetaVolumeContainer(std::string arrayName);
    bool _IsVolumeContainerExist(std::string arrayName);
    void _InsertVolumeContainer(std::string arrayName);
    bool _RemoveVolumeContainer(std::string arrayName);

    BitMap* volumeBitmap;
    std::unordered_map<std::string, uint32_t> volumeMap;
    MetaVolumeContainer* volumeContainer[MetaFsConfig::MAX_ARRAY_CNT];
};
} // namespace pos
