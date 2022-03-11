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

#include "meta_file_util.h"
#include "meta_storage_specific.h"
#include "metafs_control_request.h"
#include "src/logger/logger.h"
#include "src/metafs/mvm/volume/nvram/nvram_meta_volume.h"
#include "src/metafs/mvm/volume/ssd/ssd_meta_volume.h"

namespace pos
{
MetaVolumeContainer::MetaVolumeContainer(const int arrayId)
: nvramMetaVolAvailable(false),
  allVolumesOpened(false),
  arrayId(arrayId)
{
}

MetaVolumeContainer::~MetaVolumeContainer(void)
{
}

void
MetaVolumeContainer::InitContext(const MetaVolumeType volumeType, const int arrayId,
    const MetaLpnType maxVolPageNum, MetaStorageSubsystem* metaStorage,
    std::shared_ptr<MetaVolume> vol)
{
    std::shared_ptr<MetaVolume> volume = vol;
    if (volume == nullptr)
    {
        volume = _CreateVolume(volumeType, arrayId, maxVolPageNum, metaStorage);
    }
    volume->Init(metaStorage);
    volumeContainer.insert(std::make_pair(volumeType, volume));
}

std::shared_ptr<MetaVolume>
MetaVolumeContainer::_CreateVolume(const MetaVolumeType volumeType, const int arrayId,
    const MetaLpnType maxLpnNum, MetaStorageSubsystem* metaStorage)
{
    std::shared_ptr<MetaVolume> volume;
    if (MetaVolumeType::NvRamVolume == volumeType)
    {
        volume = std::make_shared<NvRamMetaVolume>(arrayId, maxLpnNum);
        nvramMetaVolAvailable = true;
    }
    else
    {
        volume = std::make_shared<SsdMetaVolume>(arrayId, volumeType, maxLpnNum);
    }

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "MetaVolume has been instantiated, arrayId: {}, volumeType: {}, maxLpnNum: {}",
        arrayId, (int)volumeType, maxLpnNum);

    return volume;
}

bool
MetaVolumeContainer::CreateVolume(const MetaVolumeType volumeType)
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
        std::shared_ptr<MetaVolume> volume = item.second;
        MetaVolumeType volumeType = volume->GetVolumeType();
        if (volumeType == MetaVolumeType::SsdVolume)
        {
            _SetBackupInfo(volume, Info);
        }

        if (false == volume->OpenVolume(Info, isNPOR))
        {
            // re-create only if nvram type
            if (MetaVolumeType::NvRamVolume == volumeType)
            {
                if (false == volume->CreateVolume())
                {
                    MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED,
                        "Failed to re-create meta volume(type: {})", (int)volumeType);
                    delete[] Info;
                    return false;
                }
                else
                {
                    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                        "Successfully re-created meta volume(type: {})", (int)volumeType);
                }
            }
            else
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED,
                    "Failed to open meta volume(type: {})", (int)volumeType);
                delete[] Info;
                return false;
            }
        }

        POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Opened meta volume(type: {})", (int)volumeType);
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
        std::shared_ptr<MetaVolume> volume = item.second;
        MetaVolumeType volumeType = volume->GetVolumeType();

        if (volumeType == MetaVolumeType::SsdVolume)
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
                _CleanUp(); // clear fd2TypeMap, key2volumeTypeMap
            }

            delete[] Info;
            return false;
        }

        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "MetaVolume has been successfully closed, arrayId: {}, volumeType: {}",
            arrayId, (int)volumeType);
    }

    _CleanUp(); // clear fd2TypeMap, key2volumeTypeMap
    allVolumesOpened = false;

    delete[] Info;
    return true;
}

void
MetaVolumeContainer::_SetBackupInfo(std::shared_ptr<MetaVolume> volume, MetaLpnType* info)
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

POS_EVENT_ID
MetaVolumeContainer::DetermineVolumeToCreateFile(FileSizeType fileByteSize,
    MetaFilePropertySet& prop, const MetaVolumeType volumeType)
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
MetaVolumeContainer::IsGivenVolumeExist(const MetaVolumeType volumeType)
{
    auto search = volumeContainer.find(volumeType);
    if (volumeContainer.end() == search)
    {
        return false;
    }
    return true;
}

bool
MetaVolumeContainer::TrimData(const MetaVolumeType volumeType, MetaFsFileControlRequest& reqMsg)
{
    return volumeContainer[volumeType]->TrimData(reqMsg);
}

bool
MetaVolumeContainer::CreateFile(const MetaVolumeType volumeType, MetaFsFileControlRequest& reqMsg)
{
    auto result = volumeContainer[volumeType]->CreateFile(reqMsg);

    return (result.second == POS_EVENT_ID::SUCCESS);
}

bool
MetaVolumeContainer::DeleteFile(const MetaVolumeType volumeType, MetaFsFileControlRequest& reqMsg)
{
    auto result = volumeContainer[volumeType]->DeleteFile(reqMsg);

    return (result.second == POS_EVENT_ID::SUCCESS);
}

MetaLpnType
MetaVolumeContainer::GetMaxLpn(const MetaVolumeType volumeType)
{
    return volumeContainer[volumeType]->GetMaxLpn();
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
MetaVolumeContainer::GetInodeList(std::vector<MetaFileInfoDumpCxt>*& fileInfoList, const MetaVolumeType volumeType)
{
    volumeContainer[volumeType]->GetInodeList(fileInfoList);
}

bool
MetaVolumeContainer::CopyInodeToInodeInfo(FileDescriptorType fd,
    const MetaVolumeType volumeType, MetaFileInodeInfo* inodeInfo /* output */)
{
    return volumeContainer[volumeType]->CopyInodeToInodeInfo(fd, inodeInfo);
}

POS_EVENT_ID
MetaVolumeContainer::LookupMetaVolumeType(FileDescriptorType fd, const MetaVolumeType volumeType)
{
    auto name = volumeContainer[volumeType]->LookupNameByDescriptor(fd);
    if (name != "")
        return POS_EVENT_ID::SUCCESS;

    return POS_EVENT_ID::MFS_INVALID_PARAMETER;
}

POS_EVENT_ID
MetaVolumeContainer::LookupMetaVolumeType(std::string& fileName, const MetaVolumeType volumeType)
{
    auto fd = volumeContainer[volumeType]->LookupDescriptorByName(fileName);
    if (fd != MetaFsCommonConst::INVALID_FD)
        return POS_EVENT_ID::SUCCESS;

    return POS_EVENT_ID::MFS_INVALID_PARAMETER;
}

bool
MetaVolumeContainer::CheckFileInActive(const MetaVolumeType volumeType, const FileDescriptorType fd)
{
    return volumeContainer[volumeType]->CheckFileInActive(fd);
}

POS_EVENT_ID
MetaVolumeContainer::AddFileInActiveList(const MetaVolumeType volumeType, const FileDescriptorType fd)
{
    return volumeContainer[volumeType]->AddFileInActiveList(fd);
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
MetaVolumeContainer::RemoveFileFromActiveList(const MetaVolumeType volumeType, const FileDescriptorType fd)
{
    volumeContainer[volumeType]->RemoveFileFromActiveList(fd);
}

FileSizeType
MetaVolumeContainer::GetFileSize(const MetaVolumeType volumeType, const FileDescriptorType fd)
{
    return volumeContainer[volumeType]->GetFileSize(fd);
}

FileSizeType
MetaVolumeContainer::GetDataChunkSize(const MetaVolumeType volumeType, const FileDescriptorType fd)
{
    return volumeContainer[volumeType]->GetDataChunkSize(fd);
}

MetaLpnType
MetaVolumeContainer::GetFileBaseLpn(const MetaVolumeType volumeType, const FileDescriptorType fd)
{
    return volumeContainer[volumeType]->GetFileBaseLpn(fd);
}

MetaFileInode&
MetaVolumeContainer::GetInode(const FileDescriptorType fd, const MetaVolumeType volumeType)
{
    return volumeContainer[volumeType]->GetInode(fd);
}

size_t
MetaVolumeContainer::GetAvailableSpace(const MetaVolumeType volumeType)
{
    return volumeContainer[volumeType]->GetAvailableSpace();
}

MetaLpnType
MetaVolumeContainer::GetTheLastValidLpn(const MetaVolumeType volumeType)
{
    return volumeContainer[volumeType]->GetTheLastValidLpn();
}
} // namespace pos
