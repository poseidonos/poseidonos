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

#include "meta_file_mgr.h"

#include "mfs_io_config.h"
#include "mfs_log.h"
#include "src/include/memory.h"
using namespace ibofos;

MetaFileMgrClass::MetaFileMgrClass(void)
: isAllocated(false),
  mfssIntf(nullptr)
{
}

MetaFileMgrClass::~MetaFileMgrClass(void)
{
}

void
MetaFileMgrClass::Init(MetaVolumeType volumeType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    OnVolumeMetaRegionMgr::Init(volumeType, baseLpn, maxLpn);
    extentMgr.Init(baseLpn, maxLpn);
    isAllocated = false;

    mfssIntf = metaStorage;
}

MetaLpnType
MetaFileMgrClass::GetRegionSizeInLpn(void)
{
    assert(maxLpn > baseLpn);
    return maxLpn - baseLpn;
}

void
MetaFileMgrClass::Bringup(void)
{
    // build internal data based on loadded content
}

void
MetaFileMgrClass::Finalize(void)
{
    activeFiles.clear();
    flock.clear();
}

MetaFilePageMap
MetaFileMgrClass::AllocExtent(FileSizeType fileSize)
{
    // lpnCnt => based on Data Chunk Size
    FileSizeType userDataChunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    MetaLpnType lpnCnt = (fileSize + userDataChunkSize - 1) / userDataChunkSize;

    MFS_TRACE_DEBUG(IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "AllocExtent volumeType={}, fileSize={}, lpnCnt={}",
        (int)volumeType, fileSize, lpnCnt);

    MetaFilePageMap pagemap = extentMgr.AllocExtent(lpnCnt);

    isAllocated = true;

    return pagemap;
}

void
MetaFileMgrClass::RemoveExtent(MetaLpnType baseLpn, FileSizeType fileSize)
{
    MFS_TRACE_DEBUG(IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "RemoveExtent volumeType={}, baseLpn={}, fileSize={}",
        (int)volumeType, baseLpn, fileSize);

    extentMgr.RemoveFromFreeExtentsList(baseLpn, fileSize);
    extentMgr.PrintFreeExtentsList();
}

IBOF_EVENT_ID
MetaFileMgrClass::AddFileInActiveList(FileFDType fd)
{
    IBOF_EVENT_ID rc = IBOF_EVENT_ID::SUCCESS;

    if (activeFiles.find(fd) != activeFiles.end())
    {
        rc = IBOF_EVENT_ID::MFS_FILE_OPEN_REPETITIONARY;
        IBOF_TRACE_ERROR((int)rc,
            "You attempt to open fd={} file twice. It is not allowed", fd);
    }
    else
    {
        activeFiles.insert(fd);
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "File descriptor={} is added in active fd list", fd);
    }

    return rc;
}

void
MetaFileMgrClass::RemoveFileFromActiveList(FileFDType fd)
{
    assert(activeFiles.find(fd) != activeFiles.end());
    activeFiles.erase(fd);
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "File descriptor={} is removed from active fd list, remained activeFiles={}",
        fd, activeFiles.size());
}

bool
MetaFileMgrClass::CheckFileInActive(FileFDType fd)
{
    if (activeFiles.find(fd) == activeFiles.end())
    {
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "File descriptor={} is not found in active fd list", fd);
        return false;
    }
    return true;
}

bool
MetaFileMgrClass::IsLockAlreadyGranted(FileFDType fd)
{
    auto item = flock.find(fd);
    return (flock.end() == item) ? false : true;
}

void
MetaFileMgrClass::LockFile(FileFDType fd, MDFileLockTypeEnum lock)
{
    assert(flock.find(fd) == flock.end());
    flock.insert(std::make_pair(fd, lock));
}
void
MetaFileMgrClass::UnlockFile(FileFDType fd)
{
    assert(flock.find(fd) != flock.end());
    flock.erase(fd);
}

void
MetaFileMgrClass::GetExtentContent(MetaFileExtentContent* list)
{
    extentMgr.GetContent(list);
}

void
MetaFileMgrClass::SetExtentContent(MetaFileExtentContent* list)
{
    extentMgr.SetContent(list);
}

uint32_t
MetaFileMgrClass::GetUtilizationInPercent(void)
{
    return ((maxLpn - extentMgr.GetAvailableLpnCount()) * 100) / maxLpn;
}

const FileFDSet&
MetaFileMgrClass::GetFDSetOfActiveFiles(void)
{
    return activeFiles;
}

size_t
MetaFileMgrClass::GetTheBiggestExtentSize(void)
{
    return extentMgr.GetTheBiggestExtentSize() * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
}

bool
MetaFileMgrClass::IsAllocated(void)
{
    return isAllocated;
}

bool
MetaFileMgrClass::TrimData(MetaStorageType media, MetaLpnType startLpn, MetaLpnType numTrimLpns)
{
    void* buf = nullptr;

    IBOF_EVENT_ID ret = IBOF_EVENT_ID::SUCCESS;
    ret = mfssIntf->TrimFileData(media, startLpn, buf, numTrimLpns);

    if (ret != IBOF_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "MFS file trim has been failed with NVMe Admin TRIM CMD.");

        const FileSizeType pageSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
        FileSizeType fileSize = numTrimLpns * pageSize;

        buf = Memory<pageSize>::Alloc(fileSize / pageSize);
        assert(buf != nullptr);

        // write all zeros
        ret = mfssIntf->WritePage(media, startLpn, buf, numTrimLpns); // should be async.

        if (ret != IBOF_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
                "MFS file trim has been failed with zero writing.");

            Memory<>::Free(buf);
            return false;
        }
        Memory<>::Free(buf);
    }

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "MFS file trim has been deleted!!!");

    return true;
}
