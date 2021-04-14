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
#include "meta_volume_context.h"
#include "metafs_log.h"

namespace pos
{
MetaVolumeContext::MetaVolumeContext(void)
: maxFileSizeLpnLimit(0)
{
}

MetaVolumeContext::~MetaVolumeContext(void)
{
}

void
MetaVolumeContext::InitContext(MetaVolumeType volumeType, std::string arrayName, MetaLpnType maxVolPageNum)
{
    MetaVolume* volume = _InitVolume(volumeType, arrayName, maxVolPageNum);
    volumeMap.RegisterVolumeInstance(volumeType, arrayName, volume);

    // TODO munseop.lim
    _SetGlobalMaxFileSizeLimit(maxVolPageNum);
}

bool
MetaVolumeContext::CreateVolume(MetaVolumeType volumeType, std::string arrayName)
{
    MetaVolume& volume = volumeMap.GetMetaVolume(volumeType, arrayName);

    if (false == volume.CreateNewVolume())
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED,
            "Meta volume creation has been failed.");

        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta volume has been created (volumeType={})",
        (uint32_t)volumeType);

    return true;
}

bool
MetaVolumeContext::Open(bool isNPOR, std::string arrayName)
{
    bool isSuccess = volumeMap.OpenAllVolumes(isNPOR, arrayName);

    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED,
            "Meta volume open is failed. Volume is corrupted or isn't created yet");

        return false;
    }

    _BuildFreeFDMap(arrayName);
    _BuildFileNameLookupTable(arrayName);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "All volumes have been opened.");

    return true;
}

bool
MetaVolumeContext::Close(bool& resetCxt, std::string arrayName)
{
    bool isSuccess = volumeMap.CloseAllVolumes(resetCxt /* output */, arrayName);

    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
            "Meta volume close has been failed.");
    }
    else
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Meta volume has been closed successfully.");
    }

    return isSuccess;
}

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaVolumeContext::Compaction(bool isNPOR, std::string arrayName)
{
    return volumeMap.Compaction(arrayName);
}
#endif

bool
MetaVolumeContext::IsFileInodeExist(std::string& fileName, std::string& arrayName)
{
    bool isExist = volumeMap.IsGivenFileCreated(fileName, arrayName);

    return isExist;
}

MetaLpnType
MetaVolumeContext::GetGlobalMaxFileSizeLimit(void)
{
    return maxFileSizeLpnLimit;
}

FileSizeType
MetaVolumeContext::CalculateDataChunkSizeInPage(MetaFilePropertySet& prop)
{
    FileSizeType dataChunkSizeInMetaPage = 0;
    if (MetaFileIntegrityType::Lvl0_Disable == prop.integrity)
    {
        dataChunkSizeInMetaPage = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    }
    else
    {
        // FIXME: when DI function gets supported
        assert(false);
    }

    return dataChunkSizeInMetaPage;
}

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeContext::DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop, std::string arrayName)
{
    return volumeMap.IsPossibleToCreateFile(fileByteSize, prop, arrayName);
}

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeContext::LookupMetaVolumeType(FileDescriptorType fd, std::string arrayName)
{
    return volumeMap.FindMetaVolumeType(fd, arrayName);
}

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeContext::LookupMetaVolumeType(std::string& fileName, std::string arrayName)
{
    return volumeMap.FindMetaVolumeType(fileName, arrayName);
}

MetaVolume&
MetaVolumeContext::GetMetaVolume(MetaVolumeType volType, std::string arrayName)
{
    return volumeMap.GetMetaVolume(volType, arrayName);
}

bool
MetaVolumeContext::IsGivenVolumeExist(MetaVolumeType volType, std::string arrayName)
{
    return volumeMap.IsGivenVolumeExist(volType, arrayName);
}

bool
MetaVolumeContext::GetVolOpenFlag(std::string arrayName)
{
    return volumeMap.GetVolOpenFlag(arrayName);
}

MetaLpnType
MetaVolumeContext::GetMaxMetaLpn(MetaVolumeType volType, std::string arrayName)
{
    return volumeMap.GetMaxMetaLpn(volType, arrayName);
}

size_t
MetaVolumeContext::GetTheBiggestExtentSize(MetaVolumeType volType, std::string arrayName)
{
    MetaFileManager& fileMgr = _GetFileManager(volType, arrayName);
    return fileMgr.GetTheBiggestExtentSize();
}

bool
MetaVolumeContext::CheckFileInActive(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName)
{
    MetaFileManager& fileMgr = _GetFileManager(volType, arrayName);
    return fileMgr.CheckFileInActive(fd);
}

POS_EVENT_ID
MetaVolumeContext::AddFileInActiveList(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName)
{
    MetaFileManager& fileMgr = _GetFileManager(volType, arrayName);
    return fileMgr.AddFileInActiveList(fd);
}

void
MetaVolumeContext::RemoveFileFromActiveList(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName)
{
    MetaFileManager& fileMgr = _GetFileManager(volType, arrayName);
    fileMgr.RemoveFileFromActiveList(fd);
}

bool
MetaVolumeContext::TrimData(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    FileDescriptorType fd = LookupFileDescByName(*reqMsg.fileName, *reqMsg.arrayName);
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType, *reqMsg.arrayName);
    MetaFileInode& inode = inodeMgr.GetFileInode(fd, *reqMsg.arrayName);
    MetaFilePageMap pageMap = inode.GetInodePageMap();
    MetaFileManager& fileMgr = _GetFileManager(volType, *reqMsg.arrayName);

    return fileMgr.TrimData(inode.GetStorageType(), pageMap.baseMetaLpn,
        pageMap.pageCnt);
}

bool
MetaVolumeContext::CreateFileInode(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType, *reqMsg.arrayName);
    FileDescriptorType newFD = _AllocFileDescriptor(*reqMsg.fileName, *reqMsg.arrayName);
    MetaFilePageMap pageMap = _AllocExtent(volType, reqMsg.fileByteSize, *reqMsg.arrayName);
    FileSizeType dataChunkSizeInMetaPage = CalculateDataChunkSizeInPage(reqMsg.fileProperty);
    _CopyExtentContent(volType, *reqMsg.arrayName);

    bool isSuccess = inodeMgr.CreateFileInode(reqMsg, newFD,
        pageMap, dataChunkSizeInMetaPage);

    if (true != isSuccess)
    {
        return false;
    }

    StringHashType fnameHashKey = MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName);
    _UpdateVolumeLookupInfo(fnameHashKey, newFD, *reqMsg.arrayName, volType);
    _InsertFileDescLookupHash(*reqMsg.fileName, newFD, *reqMsg.arrayName);

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Metadata File] Create volType={}, fd={}, fileName={}, startLpn={}, count={}",
        (int)volType, newFD, *reqMsg.fileName, pageMap.baseMetaLpn, pageMap.pageCnt);

    return true;
}

bool
MetaVolumeContext::DeleteFileInode(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    StringHashType fnameHashKey = MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName);
    FileDescriptorType fd = LookupFileDescByName(*reqMsg.fileName, *reqMsg.arrayName);

    MetaLpnType baseLpn = GetFileBaseLpn(volType, fd, *reqMsg.arrayName);
    MetaLpnType count = GetFileSize(volType, fd, *reqMsg.arrayName) / GetDataChunkSize(volType, fd, *reqMsg.arrayName);

    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType, *reqMsg.arrayName);

    if (true != inodeMgr.DeleteFileInode(fd, *reqMsg.arrayName))
    {
        return false;
    }

    // delete fd in volumeContainer LookupTable
    _RemoveVolumeLookupInfo(fnameHashKey, fd, *reqMsg.arrayName);
    _FreeFileDescriptor(*reqMsg.fileName, fd, *reqMsg.arrayName);
    _RemoveExtent(volType, baseLpn, count, *reqMsg.arrayName);

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Metadata File] Delete volType={}, fd={}, fileName={}, startLpn={}, count={}",
        (int)volType, fd, *reqMsg.fileName, baseLpn, count);

    return true;
}

FileSizeType
MetaVolumeContext::GetFileSize(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName)
{
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType, arrayName);
    return inodeMgr.GetFileSize(fd, arrayName);
}

FileSizeType
MetaVolumeContext::GetDataChunkSize(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName)
{
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType, arrayName);
    return inodeMgr.GetDataChunkSize(fd, arrayName);
}

MetaLpnType
MetaVolumeContext::GetFileBaseLpn(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName)
{
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType, arrayName);
    return inodeMgr.GetFileBaseLpn(fd, arrayName);
}

FileDescriptorType
MetaVolumeContext::LookupFileDescByName(std::string& fileName, std::string arrayName)
{
    return volumeMap.FindFDByName(fileName, arrayName);
}

MetaVolume*
MetaVolumeContext::_InitVolume(MetaVolumeType volType, std::string arrayName, MetaLpnType maxLpnNum)
{
    MetaVolume* volume;
    if (MetaVolumeType::SsdVolume == volType)
    {
        volume = new SsdMetaVolume(arrayName, maxLpnNum);
    }
    else if (MetaVolumeType::NvRamVolume == volType)
    {
        volume = new NvRamMetaVolume(arrayName, maxLpnNum);
    }
    else
    {
        MFS_TRACE_CRITICAL((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Invalid volume type given!");
        assert(false);
    }

    volume->Init();

    return volume;
}

void
MetaVolumeContext::_SetGlobalMaxFileSizeLimit(MetaLpnType maxVolumeLpn)
{
    if (maxFileSizeLpnLimit < maxVolumeLpn)
    {
        maxFileSizeLpnLimit = maxVolumeLpn * 100 / 95;
    }
}

FileDescriptorType
MetaVolumeContext::_AllocFileDescriptor(std::string& fileName, std::string& arrayName)
{
    return volumeMap.AllocFileDescriptor(fileName, arrayName);
}

void
MetaVolumeContext::_FreeFileDescriptor(std::string& fileName, FileDescriptorType fd, std::string& arrayName)
{
    volumeMap.FreeFileDescriptor(fileName, fd, arrayName);
}

MetaFilePageMap
MetaVolumeContext::_AllocExtent(MetaVolumeType volType, FileSizeType fileSize, std::string& arrayName)
{
    MetaFileManager& fileMgr = _GetFileManager(volType, arrayName);
    return fileMgr.AllocExtent(fileSize);
}

void
MetaVolumeContext::_UpdateVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd,
                        std::string arrayName, MetaVolumeType volumeType)
{
    volumeMap.UpdateVolumeLookupInfo(fileHashKey, fd, arrayName, volumeType);
}

void
MetaVolumeContext::_RemoveVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd,
                        std::string arrayName)
{
    volumeMap.RemoveVolumeLookupInfo(fileHashKey, fd, arrayName);
}

void
MetaVolumeContext::_RemoveExtent(MetaVolumeType volType, MetaLpnType baseLpn, FileSizeType fileSize, std::string& arrayName)
{
    MetaFileManager& fileMgr = _GetFileManager(volType, arrayName);
    fileMgr.RemoveExtent(baseLpn, fileSize);
}

void
MetaVolumeContext::_InsertFileDescLookupHash(std::string& fileName, FileDescriptorType fd, std::string& arrayName)
{
    volumeMap.InsertFileDescLookupHash(fileName, fd, arrayName);
}

bool
MetaVolumeContext::_CopyExtentContent(MetaVolumeType volType, std::string arrayName)
{
    MetaVolume& volume = volumeMap.GetMetaVolume(volType, arrayName);
    return volume.CopyExtentContent();
}

void
MetaVolumeContext::_BuildFreeFDMap(std::string arrayName)
{
    volumeMap.AddAllFDsInFreeFDMap(arrayName);
    volumeMap.BuildFreeFDMap(arrayName);
}

void
MetaVolumeContext::_BuildFileNameLookupTable(std::string arrayName)
{
    volumeMap.BuildFDLookup(arrayName);
}

MetaFileManager&
MetaVolumeContext::_GetFileManager(MetaVolumeType volType, std::string arrayName)
{
    MetaVolume& volume = volumeMap.GetMetaVolume(volType, arrayName);
    MetaFileManager& fileMgr = volume.GetFileInstance();
    return fileMgr;
}

MetaFileInodeManager&
MetaVolumeContext::_GetInodeManager(MetaVolumeType volType, std::string arrayName)
{
    MetaVolume& volume = volumeMap.GetMetaVolume(volType, arrayName);
    MetaFileInodeManager& inodeMgr = volume.GetInodeInstance();
    return inodeMgr;
}
} // namespace pos
