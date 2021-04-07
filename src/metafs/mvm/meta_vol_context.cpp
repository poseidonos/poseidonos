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

#include "meta_vol_context.h"

MetaVolContext::MetaVolContext(void)
: maxFileSizeLpnLimit(0)
{
}

MetaVolContext::~MetaVolContext(void)
{
}

void
MetaVolContext::InitVolume(MetaVolumeType volumeType, MetaLpnType maxVolPageNum)
{
    MetaVolumeClass* volume = _InitVolume(volumeType, maxVolPageNum);
    volumeContainer.RegisterVolumeInstance(volumeType, volume);

    _SetGlobalMaxFileSizeLimit(maxVolPageNum);
}

bool
MetaVolContext::CreateVolume(MetaVolumeType volumeType)
{
    MetaVolumeClass& volume = volumeContainer.GetMetaVolume(volumeType);

    if (false == volume.CreateNewVolume())
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED,
            "Meta volume creation has been failed.");

        return false;
    }

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta volume has been created (volumeType={})",
        (uint32_t)volumeType);

    return true;
}

bool
MetaVolContext::Open(bool isNPOR)
{
    bool isSuccess = volumeContainer.OpenAllVolumes(isNPOR);

    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED,
            "Meta volume open is failed. Volume is corrupted or isn't created yet");

        return false;
    }

    _BuildFreeFDMap();
    _BuildFileNameLookupTable();

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "All volumes have been opened.");

    return true;
}

bool
MetaVolContext::Close(bool& resetCxt)
{
    bool ret = false;
    bool isSuccess = volumeContainer.CloseAllVolumes(resetCxt /* output */);

    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
            "Meta volume close has been failed.");

        if (resetCxt == true)
        {
            fdMgr.Reset(); // FDMap, filekey2FDLookupMap
        }

        return false;
    }
    else
    {
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Meta volume has been closed successfully.");

        fdMgr.Reset(); // FDMap, filekey2FDLookupMap

        return true;
    }

    return ret;
}

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaVolContext::Compaction(bool isNPOR)
{
    return volumeContainer.Compaction();
}
#endif

bool
MetaVolContext::IsFileInodeExist(std::string& fileName)
{
    StringHashType fileKey = MetaFsUtilLib::GetHashKeyFromFileName(fileName);
    bool isExist = fdMgr.IsGivenFileCreated(fileKey);

    return isExist;
}

MetaLpnType
MetaVolContext::GetGlobalMaxFileSizeLimit(void)
{
    return maxFileSizeLpnLimit;
}

FileSizeType
MetaVolContext::CalculateDataChunkSizeInPage(MetaFilePropertySet& prop)
{
    FileSizeType dataChunkSizeInMetaPage = 0;
    if (MDFilePropIntegrity::Lvl0_Disable == prop.integrity)
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

std::pair<MetaVolumeType, IBOF_EVENT_ID>
MetaVolContext::DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop)
{
    return volumeContainer.DetermineVolumeToCreateFile(fileByteSize, prop);
}

std::pair<MetaVolumeType, IBOF_EVENT_ID>
MetaVolContext::LookupMetaVolumeType(FileFDType fd)
{
    return volumeContainer.LookupMetaVolumeType(fd);
}

std::pair<MetaVolumeType, IBOF_EVENT_ID>
MetaVolContext::LookupMetaVolumeType(std::string& fileName)
{
    return volumeContainer.LookupMetaVolumeType(fileName);
}

MetaVolumeClass&
MetaVolContext::GetMetaVolume(MetaVolumeType volumeType)
{
    return volumeContainer.GetMetaVolume(volumeType);
}

bool
MetaVolContext::IsGivenVolumeExist(MetaVolumeType volumeType)
{
    return volumeContainer.IsGivenVolumeExist(volumeType);
}

bool
MetaVolContext::GetVolOpenFlag(void)
{
    return volumeContainer.GetVolOpenFlag();
}

MetaLpnType
MetaVolContext::GetMaxMetaLpn(MetaVolumeType mediaType)
{
    return volumeContainer.GetMetaVolume(mediaType).GetMaxLpn();
}

size_t
MetaVolContext::GetTheBiggestExtentSize(MetaVolumeClass& tgtMetaVol)
{
    return tgtMetaVol.GetFileInstance().GetTheBiggestExtentSize();
}

bool
MetaVolContext::CheckFileInActive(MetaVolumeClass& tgtMetaVol, FileFDType fd)
{
    return tgtMetaVol.GetFileInstance().CheckFileInActive(fd);
}

bool
MetaVolContext::IsLockAlreadyGranted(MetaVolumeClass& tgtMetaVol, FileFDType fd)
{
    return tgtMetaVol.GetFileInstance().IsLockAlreadyGranted(fd);
}

void
MetaVolContext::LockFile(MetaVolumeClass& tgtMetaVol, FileFDType fd, MDFileLockTypeEnum lock)
{
    tgtMetaVol.GetFileInstance().LockFile(fd, lock);
}

void
MetaVolContext::UnlockFile(MetaVolumeClass& tgtMetaVol, FileFDType fd)
{
    tgtMetaVol.GetFileInstance().UnlockFile(fd);
}

IBOF_EVENT_ID
MetaVolContext::AddFileInActiveList(MetaVolumeClass& tgtMetaVol, FileFDType fd)
{
    return tgtMetaVol.GetFileInstance().AddFileInActiveList(fd);
}

void
MetaVolContext::RemoveFileFromActiveList(MetaVolumeClass& tgtMetaVol, FileFDType fd)
{
    tgtMetaVol.GetFileInstance().RemoveFileFromActiveList(fd);
}

bool
MetaVolContext::TrimData(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    FileFDType fd = LookupFileDescByName(*reqMsg.fileName);
    MetaFileInode& inode = _GetFileInode(tgtMetaVol, fd);
    MetaFilePageMap pageMap = inode.GetInodePageMap();

    return tgtMetaVol.GetFileInstance().TrimData(inode.GetStorageType(),
        pageMap.baseMetaLpn, pageMap.pageCnt);
}

bool
MetaVolContext::CreateFileInode(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    FileFDType newFD = _AllocNewFD(*reqMsg.fileName);
    MetaFilePageMap pageMap = _AllocExtent(tgtMetaVol, reqMsg.fileByteSize);
    FileSizeType dataChunkSizeInMetaPage = CalculateDataChunkSizeInPage(reqMsg.fileProperty);
    _CopyExtentContent(tgtMetaVol);

    bool isSuccess = tgtMetaVol.GetInodeInstance().CreateFileInode(reqMsg, newFD,
        pageMap, dataChunkSizeInMetaPage);

    if (true != isSuccess)
    {
        return false;
    }

    StringHashType fnameHashKey = MetaFsUtilLib::GetHashKeyFromFileName(*reqMsg.fileName);
    _UpdateVolumeLookupInfo(fnameHashKey, newFD, tgtMetaVol.GetVolumeType());
    _InsertFileDescLookupHash(*reqMsg.fileName, newFD);

    return true;
}

bool
MetaVolContext::DeleteFileInode(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    StringHashType fnameHashKey = MetaFsUtilLib::GetHashKeyFromFileName(*reqMsg.fileName);
    FileFDType fd = LookupFileDescByName(*reqMsg.fileName);

    MetaLpnType baseLpn = GetFileBaseLpn(tgtMetaVol, fd);
    MetaLpnType count = GetFileSize(tgtMetaVol, fd) / GetDataChunkSize(tgtMetaVol, fd);

    if (true != tgtMetaVol.GetInodeInstance().DeleteFileInode(fd))
    {
        return false;
    }

    // delete fd in volumeContainer LookupTable
    _RemoveVolumeLookupInfo(fnameHashKey, fd);
    _FreeFD(*reqMsg.fileName, fd);
    _RemoveExtent(tgtMetaVol, baseLpn, count);

    return true;
}

FileSizeType
MetaVolContext::GetFileSize(MetaVolumeClass& tgtMetaVol, FileFDType fd)
{
    return tgtMetaVol.GetInodeInstance().GetFileSize(fd);
}

FileSizeType
MetaVolContext::GetDataChunkSize(MetaVolumeClass& tgtMetaVol, FileFDType fd)
{
    return tgtMetaVol.GetInodeInstance().GetDataChunkSize(fd);
}

MetaLpnType
MetaVolContext::GetFileBaseLpn(MetaVolumeClass& tgtMetaVol, FileFDType fd)
{
    return tgtMetaVol.GetInodeInstance().GetFileBaseLpn(fd);
}

FileFDType
MetaVolContext::LookupFileDescByName(std::string& fileName)
{
    StringHashType fileKey = MetaFsUtilLib::GetHashKeyFromFileName(fileName);
    FileFDType fd = fdMgr.FindFDByName(fileKey);

    return fd;
}

MetaVolumeClass*
MetaVolContext::_InitVolume(MetaVolumeType volType, MetaLpnType maxLpnNum)
{
    MetaVolumeClass* volume;
    if (MetaVolumeType::SsdVolume == volType)
    {
        volume = new SsdMetaVolumeClass(maxLpnNum);
    }
    else if (MetaVolumeType::NvRamVolume == volType)
    {
        volume = new NvRamMetaVolumeClass(maxLpnNum);
        volumeContainer.SetNvRamVolumeAvailable();
    }
    else
    {
        MFS_TRACE_CRITICAL((int)IBOF_EVENT_ID::MFS_INVALID_PARAMETER,
            "Invalid volume type given!");
        assert(false);
    }

    volume->Init();

    return volume;
}

void
MetaVolContext::_SetGlobalMaxFileSizeLimit(MetaLpnType maxVolumeLpn)
{
    if (maxFileSizeLpnLimit < maxVolumeLpn)
    {
        maxFileSizeLpnLimit = maxVolumeLpn * 100 / 95;
    }
}

FileFDType
MetaVolContext::_AllocNewFD(std::string& fileName)
{
    FileFDType fd = fdMgr.AllocNewFD();
    StringHashType fileKey = MetaFsUtilLib::GetHashKeyFromFileName(fileName);
    fdMgr.InsertFileDescLookupHash(fileKey, fd);

    return fd;
}

void
MetaVolContext::_FreeFD(std::string& fileName, FileFDType fd)
{
    _RemoveFileDescLookupHash(fileName);
    fdMgr.FreeFD(fd);
}

MetaFilePageMap
MetaVolContext::_AllocExtent(MetaVolumeClass& tgtMetaVol, FileSizeType fileSize)
{
    return tgtMetaVol.GetFileInstance().AllocExtent(fileSize);
}

void
MetaVolContext::_UpdateVolumeLookupInfo(StringHashType fileHashKey, FileFDType fd, MetaVolumeType volumeType)
{
    volumeContainer.UpdateVolumeLookupInfo(fileHashKey, fd, volumeType);
}

void
MetaVolContext::_RemoveVolumeLookupInfo(StringHashType fileHashKey, FileFDType fd)
{
    volumeContainer.RemoveVolumeLookupInfo(fileHashKey, fd);
}

void
MetaVolContext::_RemoveExtent(MetaVolumeClass& tgtMetaVol, MetaLpnType baseLpn, FileSizeType fileSize)
{
    tgtMetaVol.GetFileInstance().RemoveExtent(baseLpn, fileSize);
}

void
MetaVolContext::_InsertFileDescLookupHash(std::string& fileName, FileFDType fd)
{
    StringHashType fileKey = MetaFsUtilLib::GetHashKeyFromFileName(fileName);
    fdMgr.InsertFileDescLookupHash(fileKey, fd);
}

void
MetaVolContext::_RemoveFileDescLookupHash(std::string& fileName)
{
    StringHashType fileKey = MetaFsUtilLib::GetHashKeyFromFileName(fileName);
    fdMgr.EraseFileDescLookupHash(fileKey);
}

bool
MetaVolContext::_CopyExtentContent(MetaVolumeClass& tgtMetaVol)
{
    return tgtMetaVol.CopyExtentContent();
}

MetaFileInode&
MetaVolContext::_GetFileInode(MetaVolumeClass& tgtMetaVol, FileFDType fd)
{
    return tgtMetaVol.GetInodeInstance().GetFileInode(fd);
}

void
MetaVolContext::_BuildFreeFDMap(void)
{
    fdMgr.AddAllFDsInFreeFDMap();
    volumeContainer.BuildFreeFDMap(fdMgr.GetFreeFDMap());
}

void
MetaVolContext::_BuildFileNameLookupTable(void)
{
    volumeContainer.BuildFDLookup(fdMgr.GetFDLookupMap());
}
