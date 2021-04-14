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

#include <string>
#include <unordered_map>
#include <utility>
#include "meta_volume_container_map.h"
#include "meta_volume_container.h"
#include "meta_volume.h"
#include "nvram_meta_volume.h"
#include "ssd_meta_volume.h"
#include "file_descriptor_manager.h"
#include "src/lib/bitmap.h"

namespace pos
{
MetaVolumeContainerMap::MetaVolumeContainerMap(void)
{
    volumeBitmap = new BitMap(MetaFsConfig::MAX_ARRAY_CNT);
    volumeBitmap->ResetBitmap();
    volumeMap.clear();

    for (uint32_t index = 0; index < MetaFsConfig::MAX_ARRAY_CNT; index++)
    {
        volumeContainer[index] = nullptr;
    }
}

MetaVolumeContainerMap::~MetaVolumeContainerMap(void)
{
    volumeMap.clear();
    volumeBitmap->ResetBitmap();
    delete volumeBitmap;

    for (uint32_t index = 0; index < MetaFsConfig::MAX_ARRAY_CNT; index++)
    {
        if (nullptr != volumeContainer[index])
            delete volumeContainer[index];
    }
}

void
MetaVolumeContainerMap::RegisterVolumeInstance(MetaVolumeType volumeType,
                            std::string arrayName, MetaVolume* volume)
{
    if (!_IsVolumeContainerExist(arrayName))
    {
        _InsertVolumeContainer(arrayName);
    }

    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    container->RegisterVolumeInstance(volumeType, volume);
}

MetaVolume&
MetaVolumeContainerMap::GetMetaVolume(MetaVolumeType volType, std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->GetMetaVolume(volType);
}

bool
MetaVolumeContainerMap::IsGivenVolumeExist(MetaVolumeType volType, std::string arrayName)
{
    if (!_IsVolumeContainerExist(arrayName))
        return false;

    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->IsGivenVolumeExist(volType);
}

bool
MetaVolumeContainerMap::IsGivenFileCreated(std::string fileName, std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    bool isExist = container->IsGivenFileCreated(fileName);
    return isExist;
}

bool
MetaVolumeContainerMap::GetVolOpenFlag(std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->GetVolOpenFlag();
}

MetaLpnType
MetaVolumeContainerMap::GetMaxMetaLpn(MetaVolumeType volType, std::string arrayName)
{
    return GetMetaVolume(volType, arrayName).GetMaxLpn();
}

bool
MetaVolumeContainerMap::OpenAllVolumes(bool isNPOR, std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->OpenAllVolumes(isNPOR);
}

bool
MetaVolumeContainerMap::CloseAllVolumes(bool& resetContext, std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    bool isSuccess = container->CloseAllVolumes(resetContext);

    if (isSuccess || resetContext)
    {
        _RemoveVolumeContainer(arrayName);
        container->ResetFileDescriptorManager();
    }

    return isSuccess;
}

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaVolumeContainerMap::Compaction(std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->Compaction();
}
#endif

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeContainerMap::IsPossibleToCreateFile(FileSizeType fileByteSize,
                            MetaFilePropertySet& prop, std::string arrayName)
{
    if (!_IsVolumeContainerExist(arrayName))
        return make_pair(MetaVolumeType::Invalid, POS_EVENT_ID::MFS_INVALID_PARAMETER);

    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->DetermineVolumeToCreateFile(fileByteSize, prop);
}

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeContainerMap::FindMetaVolumeType(FileDescriptorType fd, std::string arrayName)
{
    if (!_IsVolumeContainerExist(arrayName))
        return make_pair(MetaVolumeType::Invalid, POS_EVENT_ID::MFS_INVALID_PARAMETER);

    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->LookupMetaVolumeType(fd);
}

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeContainerMap::FindMetaVolumeType(std::string& fileName, std::string arrayName)
{
    if (!_IsVolumeContainerExist(arrayName))
        return make_pair(MetaVolumeType::Invalid, POS_EVENT_ID::MFS_INVALID_PARAMETER);

    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->LookupMetaVolumeType(fileName);
}

void
MetaVolumeContainerMap::UpdateVolumeLookupInfo(StringHashType fileHashKey,
                            FileDescriptorType fd, std::string arrayName,
                            MetaVolumeType volumeType)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    container->UpdateVolumeLookupInfo(fileHashKey, fd, volumeType);
}

void
MetaVolumeContainerMap::RemoveVolumeLookupInfo(StringHashType fileHashKey,
                            FileDescriptorType fd, std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    container->RemoveVolumeLookupInfo(fileHashKey, fd);
}

FileDescriptorType
MetaVolumeContainerMap::FindFDByName(std::string fileName, std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->FindFDByName(fileName);
}

FileDescriptorType
MetaVolumeContainerMap::AllocFileDescriptor(std::string fileName, std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    return container->AllocFileDescriptor(fileName);
}

void
MetaVolumeContainerMap::FreeFileDescriptor(std::string& fileName,
                            FileDescriptorType fd, std::string& arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    RemoveFileDescLookupHash(fileName, arrayName);
    container->FreeFileDescriptor(fd);
}

void
MetaVolumeContainerMap::AddAllFDsInFreeFDMap(std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    container->AddAllFDsInFreeFDMap();
}

void
MetaVolumeContainerMap::BuildFreeFDMap(std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    container->BuildFreeFDMap();
}

void
MetaVolumeContainerMap::BuildFDLookup(std::string arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    container->BuildFDLookup();
}

void
MetaVolumeContainerMap::InsertFileDescLookupHash(std::string& fileName,
                            FileDescriptorType fd, std::string& arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    container->InsertFileDescLookupHash(fileName, fd);
}

void
MetaVolumeContainerMap::RemoveFileDescLookupHash(std::string& fileName,
                            std::string& arrayName)
{
    MetaVolumeContainer* container = _GetMetaVolumeContainer(arrayName);
    container->EraseFileDescLookupHash(fileName);
}

MetaVolumeContainer*
MetaVolumeContainerMap::_GetMetaVolumeContainer(std::string arrayName)
{
    auto it = volumeMap.find(arrayName);
    return volumeContainer[it->second];
}

bool
MetaVolumeContainerMap::_IsVolumeContainerExist(std::string arrayName)
{
    return (volumeMap.find(arrayName) != volumeMap.end());
}

void
MetaVolumeContainerMap::_InsertVolumeContainer(std::string arrayName)
{
    uint32_t index = volumeBitmap->FindFirstZero();
    volumeBitmap->SetBit(index);
    volumeMap.insert(std::pair<std::string, uint32_t>(arrayName, index));
    volumeContainer[index] = new MetaVolumeContainer();
}

bool
MetaVolumeContainerMap::_RemoveVolumeContainer(std::string arrayName)
{
    auto it = volumeMap.find(arrayName);
    if (it == volumeMap.end())
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "The volume info has removed already, arrayName={}", arrayName);
        return false;
    }

    uint32_t index = it->second;
    volumeMap.erase(arrayName);
    volumeBitmap->ClearBit(index);

    return true;
}
} // namespace pos
