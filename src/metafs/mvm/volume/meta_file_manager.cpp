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

#include "meta_file_manager.h"

#include "metafs_config.h"
#include "metafs_log.h"
#include "src/include/memory.h"

#include <string>

namespace pos
{
MetaFileManager::MetaFileManager(std::string arrayName)
: OnVolumeMetaRegionManager(arrayName),
  isAllocated(false),
  extentMgr(new MetaFileExtentManager()),
  mfssIntf(nullptr)
{
}

MetaFileManager::~MetaFileManager(void)
{
    delete extentMgr;
}

void
MetaFileManager::Init(MetaVolumeType volumeType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    OnVolumeMetaRegionManager::Init(volumeType, baseLpn, maxLpn);
    extentMgr->Init(baseLpn, maxLpn);
    isAllocated = false;
}

void
MetaFileManager::SetMss(MetaStorageSubsystem* metaStorage)
{
    mfssIntf = metaStorage;
}

MetaLpnType
MetaFileManager::GetRegionSizeInLpn(void)
{
    assert(maxLpn > baseLpn);
    return maxLpn - baseLpn;
}

void
MetaFileManager::Bringup(void)
{
    // build internal data based on loadded content
}

void
MetaFileManager::Finalize(void)
{
    activeFiles.clear();
}

MetaFilePageMap
MetaFileManager::AllocExtent(FileSizeType fileSize)
{
    // lpnCnt => based on Data Chunk Size
    FileSizeType userDataChunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    MetaLpnType lpnCnt = (fileSize + userDataChunkSize - 1) / userDataChunkSize;
    MetaFilePageMap pagemap = extentMgr->AllocExtent(lpnCnt);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "AllocExtent volumeType={}, fileSize={}, lpnCnt={}, startLpn={}",
        (int)volumeType, fileSize, lpnCnt, pagemap.baseMetaLpn);

    isAllocated = true;

    return pagemap;
}

void
MetaFileManager::RemoveExtent(MetaLpnType baseLpn, FileSizeType fileSize)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "RemoveExtent volumeType={}, baseLpn={}, lpnCnt={}",
        (int)volumeType, baseLpn, fileSize);

    extentMgr->RemoveFromFreeExtentsList(baseLpn, fileSize);
    extentMgr->PrintFreeExtentsList();
}

POS_EVENT_ID
MetaFileManager::AddFileInActiveList(FileDescriptorType fd)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;

    if (activeFiles.find(fd) != activeFiles.end())
    {
        rc = POS_EVENT_ID::MFS_FILE_OPEN_REPETITIONARY;
        POS_TRACE_ERROR((int)rc,
            "You attempt to open fd={} file twice. It is not allowed", fd);
    }
    else
    {
        activeFiles.insert(fd);
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "File descriptor={} is added in active fd list", fd);
    }

    return rc;
}

void
MetaFileManager::RemoveFileFromActiveList(FileDescriptorType fd)
{
    assert(activeFiles.find(fd) != activeFiles.end());
    activeFiles.erase(fd);
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "File descriptor={} is removed from active fd list, remained activeFiles={}",
        fd, activeFiles.size());
}

bool
MetaFileManager::CheckFileInActive(FileDescriptorType fd)
{
    if (activeFiles.find(fd) == activeFiles.end())
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "File descriptor={} is not found in active fd list", fd);
        return false;
    }
    return true;
}

void
MetaFileManager::GetExtentContent(MetaFileExtent* list)
{
    extentMgr->GetContent(list);
}

void
MetaFileManager::SetExtentContent(MetaFileExtent* list)
{
    extentMgr->SetContent(list);
}

uint32_t
MetaFileManager::GetUtilizationInPercent(void)
{
    return ((maxLpn - extentMgr->GetAvailableLpnCount()) * 100) / maxLpn;
}

const FileDescriptorSet&
MetaFileManager::GetFDSetOfActiveFiles(void)
{
    return activeFiles;
}

size_t
MetaFileManager::GetTheBiggestExtentSize(void)
{
    return extentMgr->GetTheBiggestExtentSize() * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
}

bool
MetaFileManager::IsAllocated(void)
{
    return isAllocated;
}

bool
MetaFileManager::TrimData(MetaStorageType media, MetaLpnType startLpn, MetaLpnType numTrimLpns)
{
    void* buf = nullptr;

    POS_EVENT_ID ret = POS_EVENT_ID::SUCCESS;
    ret = mfssIntf->TrimFileData(media, startLpn, buf, numTrimLpns);

    if (ret != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "MFS file trim has been failed with NVMe Admin TRIM CMD.");

        const FileSizeType pageSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
        FileSizeType fileSize = numTrimLpns * pageSize;

        buf = Memory<pageSize>::Alloc(fileSize / pageSize);
        assert(buf != nullptr);

        // write all zeros
        ret = mfssIntf->WritePage(media, startLpn, buf, numTrimLpns); // should be async.

        if (ret != POS_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "MFS file trim has been failed with zero writing.");

            Memory<>::Free(buf);
            return false;
        }
        Memory<>::Free(buf);
    }

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "MFS file trim has been deleted!!!");

    return true;
}
} // namespace pos
