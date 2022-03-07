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

#include "meta_volume_container.h"
#include "src/metafs/mvm/volume/ssd/ssd_meta_volume.h"
#include "src/metafs/mvm/volume/nvram/nvram_meta_volume.h"
#include "meta_file_util.h"
#include "meta_storage_specific.h"
#include "metafs_control_request.h"
#include "src/logger/logger.h"

namespace pos
{
MetaVolumeContainer::MetaVolumeContainer(void)
: nvramMetaVolAvailable(false),
  allVolumesOpened(false)
{
}

MetaVolumeContainer::~MetaVolumeContainer(void)
{
    if (allVolumesOpened)
    {
        for (auto& item : volumeContainer)
        {
            MetaVolume* volume = item.second;
            delete volume;
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "volume destructed: volume={}", (uint32_t)item.first);
        }
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "volume container destructed..");
    }
}

void
MetaVolumeContainer::InitContext(MetaVolumeType volumeType, int arrayId,
                MetaLpnType maxVolPageNum, MetaStorageSubsystem* metaStorage,
                MetaVolume* vol)
{
    MetaVolume* volume = _InitVolume(volumeType, arrayId, maxVolPageNum,
                                metaStorage, vol);
    _RegisterVolumeInstance(volumeType, volume);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "volType={}", (uint32_t)(volumeType));
}

MetaVolume*
MetaVolumeContainer::_InitVolume(MetaVolumeType volType, int arrayId,
                MetaLpnType maxLpnNum, MetaStorageSubsystem* metaStorage,
                MetaVolume* vol)
{
    MetaVolume* volume = vol;

    if (vol == nullptr)
    {
        if (MetaVolumeType::NvRamVolume == volType)
        {
            volume = new NvRamMetaVolume(arrayId, maxLpnNum);
            nvramMetaVolAvailable = true;
        }
        else
        {
            volume = new SsdMetaVolume(arrayId, volType, maxLpnNum);
        }
    }

    volume->Init(metaStorage);

    return volume;
}

bool
MetaVolumeContainer::CreateVolume(MetaVolumeType volumeType)
{
    return volumeContainer[volumeType]->CreateVolume();
}

bool
MetaVolumeContainer::OpenAllVolumes(bool isNPOR)
{
    MetaLpnType* Info = new MetaLpnType[(int)BackupInfo::Max]();

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Try to open {} meta volume(s)", volumeContainer.size());

    for (auto& item : volumeContainer)
    {
        MetaVolume* volume = item.second;
        MetaVolumeType volType = volume->GetVolumeType();
        if (volType == MetaVolumeType::SsdVolume)
        {
            _SetBackupInfo(volume, Info);
        }

        if (false == volume->OpenVolume(Info, isNPOR))
        {
            // re-create only if nvram type
            if (MetaVolumeType::NvRamVolume == volType)
            {
                if (false == volume->CreateVolume())
                {
                    MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED,
                        "Failed to re-create meta volume(type: {})", (int)volType);
                    delete[] Info;
                    return false;
                }
                else
                {
                    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                        "Successfully re-created meta volume(type: {})", (int)volType);
                }
            }
            else
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED,
                    "Failed to open meta volume(type: {})", (int)volType);
                delete[] Info;
                return false;
            }
        }

        POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Opened meta volume(type: {})", (int)volType);
    }

    allVolumesOpened = true;
    delete[] Info;
    return true;
}

bool
MetaVolumeContainer::CloseAllVolumes(bool& resetContext)
{
    if (!allVolumesOpened)
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_META_VOLUME_ALREADY_CLOSED,
            "All volume already closed. Ignore duplicate volume close");
        return true;
    }

    MetaLpnType* Info = new MetaLpnType[(int)BackupInfo::Max]();
    for (auto& item : volumeContainer)
    {
        MetaVolume* volume = item.second;
        MetaVolumeType volType = volume->GetVolumeType();

        if (volType == MetaVolumeType::SsdVolume)
        {
            _SetBackupInfo(volume, Info);
        }

        if (!volume->CloseVolume(Info, resetContext /* output */))
        {
            // due to both array stop state and active files.
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
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
MetaVolumeContainer::_SetBackupInfo(MetaVolume* volume, MetaLpnType* info)
{
    if (nvramMetaVolAvailable)
    {
        info[(uint32_t)BackupInfo::BaseLpn] = volume->GetBaseLpn();
        info[(uint32_t)BackupInfo::CatalogSize] =
            (const MetaLpnType)volume->GetRegionSizeInLpn(MetaRegionType::VolCatalog);
        info[(uint32_t)BackupInfo::InodeHdrSize] =
            (const MetaLpnType)volume->GetRegionSizeInLpn(MetaRegionType::FileInodeHdr);
        info[(uint32_t)BackupInfo::InodeTableSize] =
            (const MetaLpnType)volume->GetRegionSizeInLpn(MetaRegionType::FileInodeTable);
    }
    else
    {
        info = nullptr;
    }
}

void
MetaVolumeContainer::_CleanUp(void)
{
    volumeContainer.clear();
}

void
MetaVolumeContainer::_RegisterVolumeInstance(MetaVolumeType volType, MetaVolume* metaVol)
{
    volumeContainer.insert(std::make_pair(volType, metaVol));
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "volType={}", (uint32_t)(volType));
}

POS_EVENT_ID
MetaVolumeContainer::DetermineVolumeToCreateFile(FileSizeType fileByteSize,
                MetaFilePropertySet& prop, MetaVolumeType volumeType)
{
    auto search = volumeContainer.find(volumeType);

    if (search == volumeContainer.end())
    {
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    if (!search->second->IsOkayToStore(fileByteSize, prop))
    {
        return POS_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE;
    }

    return POS_EVENT_ID::SUCCESS;
}

bool
MetaVolumeContainer::IsGivenVolumeExist(MetaVolumeType volType)
{
    auto search = volumeContainer.find(volType);
    if (volumeContainer.end() == search)
    {
        return false;
    }
    return true;
}

bool
MetaVolumeContainer::TrimData(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    return volumeContainer[volType]->TrimData(reqMsg);
}

bool
MetaVolumeContainer::CreateFile(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    auto result = volumeContainer[volType]->CreateFile(reqMsg);

    return (result.second == POS_EVENT_ID::SUCCESS);
}

bool
MetaVolumeContainer::DeleteFile(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    auto result = volumeContainer[volType]->DeleteFile(reqMsg);

    return (result.second == POS_EVENT_ID::SUCCESS);
}

MetaLpnType
MetaVolumeContainer::GetMaxLpn(MetaVolumeType volType)
{
    return volumeContainer[volType]->GetMaxLpn();
}

FileDescriptorType
MetaVolumeContainer::LookupFileDescByName(std::string& fileName)
{
    FileDescriptorType fd = MetaFsCommonConst::INVALID_FD;
    for (auto& it : volumeContainer)
    {
        fd = it.second->LookupDescriptorByName(fileName);
        if (fd != MetaFsCommonConst::INVALID_FD)
            break;
    }
    return fd;
}

void
MetaVolumeContainer::GetInodeList(std::vector<MetaFileInfoDumpCxt>*& fileInfoList, MetaVolumeType volumeType)
{
    volumeContainer[volumeType]->GetInodeList(fileInfoList);
}

bool
MetaVolumeContainer::CopyInodeToInodeInfo(FileDescriptorType fd,
        MetaVolumeType volumeType, MetaFileInodeInfo* inodeInfo /* output */)
{
    return volumeContainer[volumeType]->CopyInodeToInodeInfo(fd, inodeInfo);
}

POS_EVENT_ID
MetaVolumeContainer::LookupMetaVolumeType(FileDescriptorType fd, MetaVolumeType volumeType)
{
    auto name = volumeContainer[volumeType]->LookupNameByDescriptor(fd);
    if (name != "")
        return POS_EVENT_ID::SUCCESS;

    return POS_EVENT_ID::MFS_INVALID_PARAMETER;
}

POS_EVENT_ID
MetaVolumeContainer::LookupMetaVolumeType(std::string& fileName, MetaVolumeType volumeType)
{
    auto fd = volumeContainer[volumeType]->LookupDescriptorByName(fileName);
    if (fd != MetaFsCommonConst::INVALID_FD)
        return POS_EVENT_ID::SUCCESS;

    return POS_EVENT_ID::MFS_INVALID_PARAMETER;
}

bool
MetaVolumeContainer::CheckFileInActive(MetaVolumeType volType, FileDescriptorType fd)
{
    return volumeContainer[volType]->CheckFileInActive(fd);
}

POS_EVENT_ID
MetaVolumeContainer::AddFileInActiveList(MetaVolumeType volType, FileDescriptorType fd)
{
    return volumeContainer[volType]->AddFileInActiveList(fd);
}

bool
MetaVolumeContainer::IsGivenFileCreated(std::string& fileName)
{
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
    bool isCreated = false;

    for (auto& vol : volumeContainer)
    {
        isCreated = vol.second->IsGivenFileCreated(fileKey);
        if (isCreated)
            break;
    }

    return isCreated;
}

void
MetaVolumeContainer::RemoveFileFromActiveList(MetaVolumeType volType, FileDescriptorType fd)
{
    volumeContainer[volType]->RemoveFileFromActiveList(fd);
}

FileSizeType
MetaVolumeContainer::GetFileSize(MetaVolumeType volType, FileDescriptorType fd)
{
    return volumeContainer[volType]->GetFileSize(fd);
}

FileSizeType
MetaVolumeContainer::GetDataChunkSize(MetaVolumeType volType, FileDescriptorType fd)
{
    return volumeContainer[volType]->GetDataChunkSize(fd);
}

MetaLpnType
MetaVolumeContainer::GetFileBaseLpn(MetaVolumeType volType, FileDescriptorType fd)
{
    return volumeContainer[volType]->GetFileBaseLpn(fd);
}

MetaFileInode&
MetaVolumeContainer::GetInode(FileDescriptorType fd, MetaVolumeType volumeType)
{
    return volumeContainer[volumeType]->GetInode(fd);
}

size_t
MetaVolumeContainer::GetAvailableSpace(MetaVolumeType volType)
{
    return volumeContainer[volType]->GetAvailableSpace();
}

MetaLpnType
MetaVolumeContainer::GetTheLastValidLpn(MetaVolumeType volumeType)
{
    return volumeContainer[volumeType]->GetTheLastValidLpn();
}
} // namespace pos
