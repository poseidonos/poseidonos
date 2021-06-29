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
MetaVolumeContext::InitContext(MetaVolumeType volumeType, int arrayId,
                MetaLpnType maxVolPageNum, MetaStorageSubsystem* metaStorage)
{
    MetaVolume* volume = _InitVolume(volumeType, arrayId, maxVolPageNum, metaStorage);
    volumeContainer.RegisterVolumeInstance(volumeType, volume);

    _SetGlobalMaxFileSizeLimit(maxVolPageNum);
}

bool
MetaVolumeContext::CreateVolume(MetaVolumeType volumeType)
{
    MetaVolume& volume = volumeContainer.GetMetaVolume(volumeType);

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
MetaVolumeContext::Open(bool isNPOR)
{
    bool isSuccess = volumeContainer.OpenAllVolumes(isNPOR);

    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED,
            "Meta volume open is failed. Volume is corrupted or isn't created yet");

        return false;
    }

    _BuildFreeFDMap();
    _BuildFileNameLookupTable();

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "All volumes have been opened.");

    return true;
}

bool
MetaVolumeContext::Close(bool& resetCxt)
{
    bool ret = false;
    bool isSuccess = volumeContainer.CloseAllVolumes(resetCxt /* output */);

    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
            "Meta volume close has been failed.");

        if (resetCxt == true)
        {
            fdMgr.Reset(); // FDMap, filekey2FDLookupMap
        }

        return false;
    }
    else
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Meta volume has been closed successfully.");

        fdMgr.Reset(); // FDMap, filekey2FDLookupMap

        return true;
    }

    return ret;
}

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaVolumeContext::Compaction(bool isNPOR)
{
    return volumeContainer.Compaction();
}
#endif

bool
MetaVolumeContext::IsFileInodeExist(std::string& fileName)
{
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
    bool isExist = fdMgr.IsGivenFileCreated(fileKey);

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
MetaVolumeContext::DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop)
{
    return volumeContainer.DetermineVolumeToCreateFile(fileByteSize, prop);
}

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeContext::LookupMetaVolumeType(FileDescriptorType fd)
{
    return volumeContainer.LookupMetaVolumeType(fd);
}

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeContext::LookupMetaVolumeType(std::string& fileName)
{
    return volumeContainer.LookupMetaVolumeType(fileName);
}

MetaVolume&
MetaVolumeContext::GetMetaVolume(MetaVolumeType volType)
{
    return volumeContainer.GetMetaVolume(volType);
}

bool
MetaVolumeContext::IsGivenVolumeExist(MetaVolumeType volType)
{
    return volumeContainer.IsGivenVolumeExist(volType);
}

bool
MetaVolumeContext::GetVolOpenFlag(void)
{
    return volumeContainer.GetVolOpenFlag();
}

MetaLpnType
MetaVolumeContext::GetMaxMetaLpn(MetaVolumeType volType)
{
    return volumeContainer.GetMetaVolume(volType).GetMaxLpn();
}

size_t
MetaVolumeContext::GetTheBiggestExtentSize(MetaVolumeType volType)
{
    MetaFileManager& fileMgr = _GetFileManager(volType);
    return fileMgr.GetTheBiggestExtentSize();
}

bool
MetaVolumeContext::CheckFileInActive(MetaVolumeType volType, FileDescriptorType fd)
{
    MetaFileManager& fileMgr = _GetFileManager(volType);
    return fileMgr.CheckFileInActive(fd);
}

POS_EVENT_ID
MetaVolumeContext::AddFileInActiveList(MetaVolumeType volType, FileDescriptorType fd)
{
    MetaFileManager& fileMgr = _GetFileManager(volType);
    return fileMgr.AddFileInActiveList(fd);
}

void
MetaVolumeContext::RemoveFileFromActiveList(MetaVolumeType volType, FileDescriptorType fd)
{
    MetaFileManager& fileMgr = _GetFileManager(volType);
    fileMgr.RemoveFileFromActiveList(fd);
}

bool
MetaVolumeContext::TrimData(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    FileDescriptorType fd = LookupFileDescByName(*reqMsg.fileName);
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType);
    MetaFileInode& inode = inodeMgr.GetFileInode(fd);
    MetaFilePageMap pageMap = inode.GetInodePageMap();
    MetaFileManager& fileMgr = _GetFileManager(volType);

    return fileMgr.TrimData(inode.GetStorageType(), pageMap.baseMetaLpn,
        pageMap.pageCnt);
}

bool
MetaVolumeContext::CreateFileInode(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType);
    FileDescriptorType newFD = _AllocFileDescriptor(*reqMsg.fileName);
    MetaFilePageMap pageMap = _AllocExtent(volType, reqMsg.fileByteSize);
    FileSizeType dataChunkSizeInMetaPage = CalculateDataChunkSizeInPage(reqMsg.fileProperty);
    _CopyExtentContent(volType);

    bool isSuccess = inodeMgr.CreateFileInode(reqMsg, newFD,
        pageMap, dataChunkSizeInMetaPage);

    if (true != isSuccess)
    {
        return false;
    }

    StringHashType fnameHashKey = MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName);
    _UpdateVolumeLookupInfo(fnameHashKey, newFD, volType);
    _InsertFileDescLookupHash(*reqMsg.fileName, newFD);

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Metadata File] Create volType={}, fd={}, fileName={}, startLpn={}, count={}",
        (int)volType, newFD, *reqMsg.fileName, pageMap.baseMetaLpn, pageMap.pageCnt);

    return true;
}

bool
MetaVolumeContext::DeleteFileInode(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    StringHashType fnameHashKey = MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName);
    FileDescriptorType fd = LookupFileDescByName(*reqMsg.fileName);

    MetaLpnType baseLpn = GetFileBaseLpn(volType, fd);
    MetaLpnType count = GetFileSize(volType, fd) / GetDataChunkSize(volType, fd);

    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType);

    if (true != inodeMgr.DeleteFileInode(fd))
    {
        return false;
    }

    // delete fd in volumeContainer LookupTable
    _RemoveVolumeLookupInfo(fnameHashKey, fd);
    _FreeFileDescriptor(*reqMsg.fileName, fd);
    _RemoveExtent(volType, baseLpn, count);

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Metadata File] Delete volType={}, fd={}, fileName={}, startLpn={}, count={}",
        (int)volType, fd, *reqMsg.fileName, baseLpn, count);

    return true;
}

FileSizeType
MetaVolumeContext::GetFileSize(MetaVolumeType volType, FileDescriptorType fd)
{
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType);
    return inodeMgr.GetFileSize(fd);
}

FileSizeType
MetaVolumeContext::GetDataChunkSize(MetaVolumeType volType, FileDescriptorType fd)
{
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType);
    return inodeMgr.GetDataChunkSize(fd);
}

MetaLpnType
MetaVolumeContext::GetFileBaseLpn(MetaVolumeType volType, FileDescriptorType fd)
{
    MetaFileInodeManager& inodeMgr = _GetInodeManager(volType);
    return inodeMgr.GetFileBaseLpn(fd);
}

FileDescriptorType
MetaVolumeContext::LookupFileDescByName(std::string& fileName)
{
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
    FileDescriptorType fd = fdMgr.FindFDByName(fileKey);

    return fd;
}

MetaVolume*
MetaVolumeContext::_InitVolume(MetaVolumeType volType, int arrayId, MetaLpnType maxLpnNum, MetaStorageSubsystem* metaStorage)
{
    MetaVolume* volume;
    if (MetaVolumeType::SsdVolume == volType)
    {
        volume = new SsdMetaVolume(arrayId, maxLpnNum);
    }
    else if (MetaVolumeType::NvRamVolume == volType)
    {
        volume = new NvRamMetaVolume(arrayId, maxLpnNum);
        volumeContainer.SetNvRamVolumeAvailable();
    }
    else
    {
        MFS_TRACE_CRITICAL((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Invalid volume type given!");
        assert(false);
    }

    volume->Init(metaStorage);

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
MetaVolumeContext::_AllocFileDescriptor(std::string& fileName)
{
    FileDescriptorType fd = fdMgr.Alloc();
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
    fdMgr.InsertFileDescLookupHash(fileKey, fd);

    return fd;
}

void
MetaVolumeContext::_FreeFileDescriptor(std::string& fileName, FileDescriptorType fd)
{
    _RemoveFileDescLookupHash(fileName);
    fdMgr.Free(fd);
}

MetaFilePageMap
MetaVolumeContext::_AllocExtent(MetaVolumeType volType, FileSizeType fileSize)
{
    MetaFileManager& fileMgr = _GetFileManager(volType);
    return fileMgr.AllocExtent(fileSize);
}

void
MetaVolumeContext::_UpdateVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd, MetaVolumeType volumeType)
{
    volumeContainer.UpdateVolumeLookupInfo(fileHashKey, fd, volumeType);
}

void
MetaVolumeContext::_RemoveVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd)
{
    volumeContainer.RemoveVolumeLookupInfo(fileHashKey, fd);
}

void
MetaVolumeContext::_RemoveExtent(MetaVolumeType volType, MetaLpnType baseLpn, FileSizeType fileSize)
{
    MetaFileManager& fileMgr = _GetFileManager(volType);
    fileMgr.RemoveExtent(baseLpn, fileSize);
}

void
MetaVolumeContext::_InsertFileDescLookupHash(std::string& fileName, FileDescriptorType fd)
{
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
    fdMgr.InsertFileDescLookupHash(fileKey, fd);
}

void
MetaVolumeContext::_RemoveFileDescLookupHash(std::string& fileName)
{
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
    fdMgr.EraseFileDescLookupHash(fileKey);
}

bool
MetaVolumeContext::_CopyExtentContent(MetaVolumeType volType)
{
    MetaVolume& volume = volumeContainer.GetMetaVolume(volType);
    return volume.CopyExtentContent();
}

void
MetaVolumeContext::_BuildFreeFDMap(void)
{
    fdMgr.AddAllFDsInFreeFDMap();
    volumeContainer.BuildFreeFDMap(fdMgr.GetFreeFDMap());
}

void
MetaVolumeContext::_BuildFileNameLookupTable(void)
{
    volumeContainer.BuildFDLookup(fdMgr.GetFDLookupMap());
}

MetaFileManager&
MetaVolumeContext::_GetFileManager(MetaVolumeType volType)
{
    MetaVolume& volume = volumeContainer.GetMetaVolume(volType);
    MetaFileManager& fileMgr = volume.GetFileInstance();
    return fileMgr;
}

MetaFileInodeManager&
MetaVolumeContext::_GetInodeManager(MetaVolumeType volType)
{
    MetaVolume& volume = volumeContainer.GetMetaVolume(volType);
    MetaFileInodeManager& inodeMgr = volume.GetInodeInstance();
    return inodeMgr;
}

} // namespace pos
