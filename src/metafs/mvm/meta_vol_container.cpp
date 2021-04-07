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

#include "meta_vol_container.h"

#include "meta_file_util.h"
#include "meta_storage_specific.h"
#include "mvm_req.h"
#include "src/logger/logger.h"

MetaVolContainerClass::MetaVolContainerClass(void)
: nvramMetaVolAvailable(false),
  allVolumesOpened(false)
{
}

MetaVolContainerClass::~MetaVolContainerClass(void)
{
    if (allVolumesOpened)
    {
        for (auto& item : volumeContainer)
        {
            MetaVolumeClass* volume = item.second;
            delete volume;
            MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
                "volume destructed: volume={}", (uint32_t)item.first);
        }
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "volume container destructed..");
    }
}

bool
MetaVolContainerClass::OpenAllVolumes(bool isNPOR)
{
    MetaLpnType* Info = new MetaLpnType[BackupInfo::Max]();

    bool isSuccess;
    for (auto& item : volumeContainer)
    {
        MetaVolumeClass* volume = item.second;
        MetaVolumeType volType = volume->GetVolumeType();
        if (volType == MetaVolumeType::SsdVolume)
        {
            _SetBackupInfo(volume, Info);
        }

        isSuccess = volume->OpenVolume(Info, isNPOR);
        if (false == isSuccess)
        {
            MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED,
                "Meta volume open failed.");
            delete[] Info;
            return false;
        }
        _BuildFD2VolumeTypeMap(volume);
        _BuildFileKey2VolumeTypeMap(volume);
    }

    allVolumesOpened = true;
    delete[] Info;
    return true;
}

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaVolContainerClass::Compaction(void)
{
    bool isSuccess = true;

    for (auto& item : volumeContainer)
    {
        MetaVolumeClass* volume = item.second;
        MetaVolumeType volType = volume->GetVolumeType();
        if (volType == MetaVolumeType::SsdVolume)
        {
            isSuccess = volume->Compaction();
        }
    }

    return isSuccess;
}
#endif

bool
MetaVolContainerClass::CloseAllVolumes(bool& resetContext)
{
    if (!allVolumesOpened)
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_META_VOLUME_ALREADY_CLOSED,
            "All volume already closed. Ignore duplicate volume close");
        return true;
    }

    MetaLpnType* Info = new MetaLpnType[BackupInfo::Max]();
    bool isSuccess;
    for (auto& item : volumeContainer)
    {
        MetaVolumeClass* volume = item.second;
        MetaVolumeType volType = volume->GetVolumeType();

        if (volType == MetaVolumeType::SsdVolume)
        {
            _SetBackupInfo(volume, Info);
        }

        isSuccess = volume->CloseVolume(Info, resetContext /* output */);
        if (false == isSuccess)
        {
            // due to both array stop state and active files.
            MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
                "Meta volume close is failed, cxt={}", resetContext);

            if (true == resetContext)
            {               // in the case of stop state return
                _CleanUp(); // clear fd2TypeMap, key2VoltypeMap
            }

            delete[] Info;
            return false;
        }

        delete volume;
    }

    _CleanUp(); // clear fd2TypeMap, key2VoltypeMap
    allVolumesOpened = false;

    delete[] Info;
    return true;
}

void
MetaVolContainerClass::_SetBackupInfo(MetaVolumeClass* volume, MetaLpnType* info)
{
    if (IsNvRamVolumeAvailable())
    {
        info[(uint32_t)BackupInfo::BaseLpn] = volume->GetBaseLpn();
        info[(uint32_t)BackupInfo::CatalogSize] =
            (const MetaLpnType)volume->GetCatalogInstance().GetRegionSizeInLpn();
        info[(uint32_t)BackupInfo::InodeHdrSize] =
            (const MetaLpnType)volume->GetInodeInstance().GetRegionSizeInLpn(MetaRegionType::FileInodeHdr);
        info[(uint32_t)BackupInfo::InodeTableSize] =
            (const MetaLpnType)volume->GetInodeInstance().GetRegionSizeInLpn(MetaRegionType::FileInodeTable);
    }
    else
    {
        info = nullptr;
    }
}

void
MetaVolContainerClass::_CleanUp(void)
{
    fd2VolTypehMap.clear();
    fileKey2VolTypeMap.clear();

    volumeContainer.clear();
}

void
MetaVolContainerClass::RegisterVolumeInstance(MetaVolumeType volType, MetaVolumeClass* metaVol)
{
    volumeContainer.insert(std::make_pair(volType, metaVol));
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "volType={}", (uint32_t)(volType));
}

void
MetaVolContainerClass::SetNvRamVolumeAvailable(void)
{
    nvramMetaVolAvailable = true;
}

bool
MetaVolContainerClass::IsNvRamVolumeAvailable(void)
{
    return nvramMetaVolAvailable;
}

std::pair<MetaVolumeType, IBOF_EVENT_ID>
MetaVolContainerClass::DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop)
{
    MetaVolumeType candidateVolType;

    if (IsNvRamVolumeAvailable())
    {
        bool isOkayToStoreNvRam = false;
        if (fileByteSize == 0)
        {
            auto nvramVolume = volumeContainer.find(MetaVolumeType::NvRamVolume);
            isOkayToStoreNvRam = (nvramVolume->second)->IsOkayToStore(fileByteSize, prop);
        }
        else
        {
            isOkayToStoreNvRam = _CheckOkayToStore(MetaVolumeType::NvRamVolume, fileByteSize, prop);
        }

        if (isOkayToStoreNvRam)
        {
            candidateVolType = MetaVolumeType::NvRamVolume;
        }
        else
        {
            candidateVolType = MetaVolumeType::SsdVolume;
        }
    }
    else
    {
        candidateVolType = MetaVolumeType::SsdVolume;
    }

    if (candidateVolType == MetaVolumeType::SsdVolume)
    { // Check free SSD space.
        bool isOkayToStoreSSD = false;
        auto ssdVolume = volumeContainer.find(MetaVolumeType::SsdVolume);
        isOkayToStoreSSD = (ssdVolume->second)->IsOkayToStore(fileByteSize, prop);

        if (isOkayToStoreSSD == false)
        {
            return make_pair(candidateVolType, IBOF_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE);
        }
    }
    return make_pair(candidateVolType, IBOF_EVENT_ID::SUCCESS);
}

bool
MetaVolContainerClass::_CheckOkayToStore(MetaVolumeType volumeType, FileSizeType fileByteSize, MetaFilePropertySet& prop)
{
    auto search = volumeContainer.find(volumeType);
    assert(search != volumeContainer.end());
    MetaVolumeClass* volume = search->second;

    return volume->IsOkayToStore(fileByteSize, prop);
}

bool
MetaVolContainerClass::IsGivenVolumeExist(MetaVolumeType volumeType)
{
    auto search = volumeContainer.find(volumeType);
    if (volumeContainer.end() == search)
    {
        return false;
    }
    return true;
}

MetaVolumeClass&
MetaVolContainerClass::GetMetaVolume(MetaVolumeType volumeType)
{
    auto search = volumeContainer.find(volumeType);
    assert(search != volumeContainer.end());

    MetaVolumeClass* metaVol = search->second;
    return *metaVol;
}

void
MetaVolContainerClass::_BuildFD2VolumeTypeMap(MetaVolumeClass* volume)
{
    volume->BuildFDMap(fd2VolTypehMap);
}

void
MetaVolContainerClass::_BuildFileKey2VolumeTypeMap(MetaVolumeClass* volume)
{
    volume->BuildFileNameMap(fileKey2VolTypeMap);
}

void
MetaVolContainerClass::BuildFreeFDMap(std::map<FileFDType, FileFDType>& freeFDMap)
{
    for (auto& item : volumeContainer)
    {
        MetaVolumeClass* volume = item.second;
        volume->BuildFreeFDMap(freeFDMap);
    }
}

void
MetaVolContainerClass::BuildFDLookup(std::unordered_map<StringHashType, FileFDType>& fileKeyLookupMap)
{
    for (auto& item : volumeContainer)
    {
        MetaVolumeClass* volume = item.second;
        volume->BuildFDLookupMap(fileKeyLookupMap);
    }
}

std::pair<MetaVolumeType, IBOF_EVENT_ID>
MetaVolContainerClass::LookupMetaVolumeType(FileFDType fd)
{
    if (fd2VolTypehMap.empty())
    {
        return make_pair(MetaVolumeType::Invalid, IBOF_EVENT_ID::MFS_FILE_NOT_FOUND);
    }

    auto item = fd2VolTypehMap.find(fd);
    if (item == fd2VolTypehMap.end())
    {
        return make_pair(MetaVolumeType::Invalid, IBOF_EVENT_ID::MFS_FILE_NOT_FOUND);
    }

    return make_pair(item->second, IBOF_EVENT_ID::SUCCESS);
}

std::pair<MetaVolumeType, IBOF_EVENT_ID>
MetaVolContainerClass::LookupMetaVolumeType(std::string& fileName)
{
    if (fileKey2VolTypeMap.empty())
    {
        return make_pair(MetaVolumeType::Invalid, IBOF_EVENT_ID::MFS_FILE_NOT_FOUND);
    }

    StringHashType hashKey = MetaFsUtilLib::GetHashKeyFromFileName(fileName);
    auto item = fileKey2VolTypeMap.find(hashKey);
    if (item == fileKey2VolTypeMap.end())
    {
        return make_pair(MetaVolumeType::Invalid, IBOF_EVENT_ID::MFS_FILE_NOT_FOUND);
    }

    return make_pair(item->second, IBOF_EVENT_ID::SUCCESS);
}

void
MetaVolContainerClass::UpdateVolumeLookupInfo(StringHashType fileHashKey, FileFDType fd, MetaVolumeType volumeType)
{
    {
        auto item = fileKey2VolTypeMap.find(fileHashKey);
        assert(fileKey2VolTypeMap.end() == item);
        fileKey2VolTypeMap.insert(std::make_pair(fileHashKey, volumeType));
    }

    {
        auto item = fd2VolTypehMap.find(fd);
        assert(fd2VolTypehMap.end() == item);
        fd2VolTypehMap.insert(std::make_pair(fd, volumeType));
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "fileHashKey: {}, fd: {}", fileHashKey, fd);
    }
}

void
MetaVolContainerClass::RemoveVolumeLookupInfo(StringHashType fileHashKey, FileFDType fd)
{
    fileKey2VolTypeMap.erase(fileHashKey);
    fd2VolTypehMap.erase(fd);
}
