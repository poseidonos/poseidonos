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
#include "mf_inode_mgr.h"
#include "meta_file_util.h"
#include "metafs_log.h"
#include "region_deque.h"

namespace pos
{
MetaFileInodeManager::MetaFileInodeManager(void)
: inodeHdr(nullptr),
  inodeTable(nullptr)
{
}

MetaFileInodeManager::~MetaFileInodeManager(void)
{
    delete inodeHdr;
    delete inodeTable;
}

void
MetaFileInodeManager::Init(std::string arrayName, MetaVolumeType volumeType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    OnVolumeMetaRegionManager::Init(arrayName, volumeType, baseLpn, maxLpn);

    MetaLpnType targetBaseLpn = baseLpn;
    if (nullptr == inodeHdr)
    {
        inodeHdr = new InodeTableHeader(volumeType, targetBaseLpn);
        targetBaseLpn += inodeHdr->GetLpnCntOfRegion();
    }
    if (nullptr == inodeTable)
    {
        inodeTable = new InodeTable(volumeType, targetBaseLpn);
        targetBaseLpn += inodeTable->GetLpnCntOfRegion();
    }
}

MetaLpnType
MetaFileInodeManager::GetRegionBaseLpn(MetaRegionType regionType)
{
    switch (regionType)
    {
        case MetaRegionType::FileInodeHdr:
        {
            return inodeHdr->GetBaseLpn();
        }
        break;
        case MetaRegionType::FileInodeTable:
        {
            return inodeTable->GetBaseLpn();
        }
        break;
        default:
            assert(false);
    }
}

MetaLpnType
MetaFileInodeManager::GetRegionSizeInLpn(MetaRegionType regionType)
{
    switch (regionType)
    {
        case MetaRegionType::FileInodeHdr:
        {
            return inodeHdr->GetLpnCntOfRegion();
        }
        break;
        case MetaRegionType::FileInodeTable:
        {
            return inodeTable->GetLpnCntOfRegion();
        }
        break;
        default:
            assert(false);
    }
}

MetaLpnType
MetaFileInodeManager::GetRegionSizeInLpn(void)
{
    MetaLpnType inodeContentSum = 0;
    inodeContentSum += inodeHdr->GetLpnCntOfRegion();
    inodeContentSum += inodeTable->GetLpnCntOfRegion();

    return inodeContentSum;
}

void
MetaFileInodeManager::Bringup(void)
{
    inodeHdr->BuildFreeInodeEntryMap();
    _BuildF2InodeMap();
}

void
MetaFileInodeManager::_BuildF2InodeMap(void)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& inodeInUseBitmap = inodeHdr->GetInodeInUseBitmap();
    uint32_t size = inodeInUseBitmap.size();
    for (uint32_t idx = 0; idx < size; idx++)
    {
        if (inodeInUseBitmap.test(idx))
        {
            MetaFileInode& inode = inodeTable->GetContent()->entries[idx];
            _UpdateFd2InodeMap(inode.data.basic.field.fd, inode);
        }
    }
}

void
MetaFileInodeManager::CreateInitialInodeContent(uint32_t maxInodeNum)
{
    inodeHdr->Create(maxInodeNum);
    inodeTable->Create(maxInodeNum);
}

bool
MetaFileInodeManager::LoadInodeContent(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Load inode header content...");

    if (true != inodeHdr->Load(arrayName))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_LOAD_FAILED,
            "Load I/O for MFS inode header has failed...");
        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Total allocated FD count = {}",
        inodeHdr->GetTotalAllocatedInodeCnt());

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Load the valid inode table content...");

    MetaStorageType media = MetaFileUtil::ConvertToMediaType(volumeType);
    MetaLpnType startBaseLpn = GetRegionBaseLpn(MetaRegionType::FileInodeTable);

    if (false == _LoadInodeFromMedia(media, startBaseLpn))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_LOAD_FAILED,
            "Load I/O for MFS inode content has failed...");

        return false;
    }

    return true;
}

/* Restore : meta in SSD volume to meta in NVRAM volume */
bool
MetaFileInodeManager::RestoreContent(MetaVolumeType targetVol, MetaLpnType baseLpn,
    MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableCnts)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Restore inode header content] from vol= {}, baseLpn={}, LpnCnts={}...",
        targetVol, baseLpn, iNodeHdrLpnCnts);

    bool isSuccess;
    MetaStorageType media = MetaFileUtil::ConvertToMediaType(targetVol);

    isSuccess = inodeHdr->Load(arrayName, media, baseLpn, 0 /*idx */, iNodeHdrLpnCnts);
    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_LOAD_FAILED,
            "Restore I/O for MFS inode header of NVRAM meta vol. has failed...");
        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Restore the valid inode table content] from baseLpn={}, LpnCnts={}...",
        baseLpn + iNodeHdrLpnCnts, iNodeTableCnts);

    MetaLpnType startBaseLpn = baseLpn + iNodeHdrLpnCnts;

    if (false == _LoadInodeFromMedia(media, startBaseLpn))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_LOAD_FAILED,
            "Restore I/O for MFS inode table of NVRAM meta vol. has failed...");

        return false;
    }

    return true;
}

bool
MetaFileInodeManager::_LoadInodeFromMedia(MetaStorageType media, MetaLpnType baseLpn)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& inodeInUseBitmap = inodeHdr->GetInodeInUseBitmap();
    uint32_t size = inodeInUseBitmap.size();
    for (uint32_t idx = 0; idx < size; idx++)
    {
        if (inodeInUseBitmap.test(idx))
        {
            if (false == inodeTable->Load(arrayName, media, baseLpn, idx, 1 /*LpnCnts*/))
            {
                return false;
            }
        }
    }

    return true;
}

bool
MetaFileInodeManager::SaveContent(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Save inode header content...");

    if (true != inodeHdr->Store(arrayName))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
            "Store I/O for MFS inode header has failed...");

        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Save the valid inode table content...");

    MetaStorageType media = MetaFileUtil::ConvertToMediaType(volumeType);
    MetaLpnType startBaseLpn = GetRegionBaseLpn(MetaRegionType::FileInodeTable);

    if (false == _StoreInodeToMedia(media, startBaseLpn))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
            "Store I/O for MFS inode table has failed...");

        return false;
    }

    return true;
}

/* Backup : meta in NVRAM volume to meta in SSD volume) */
bool
MetaFileInodeManager::BackupContent(MetaVolumeType targetVol, MetaLpnType baseLpn,
    MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableCnts)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Bakcup inode Header Content] to vol= {}, baseLpn={}, LpnCnts={}...",
        targetVol, baseLpn, iNodeHdrLpnCnts);

    bool isSuccess;
    MetaStorageType media = MetaFileUtil::ConvertToMediaType(targetVol);

    isSuccess = inodeHdr->Store(arrayName, media, baseLpn, 0 /*buf idx */, iNodeHdrLpnCnts);
    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
            "Backup I/O for MFS inode header of NVRAM meta vol. has failed...");
        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Backup the valid inode Table Content] to Backup vol= {}, baseLpn={}, LpnCnts={}...",
        targetVol, baseLpn + iNodeHdrLpnCnts, iNodeHdrLpnCnts);

    MetaLpnType startBaseLpn = baseLpn + iNodeHdrLpnCnts;

    if (false == _StoreInodeToMedia(media, startBaseLpn))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
            "Backup I/O for MFS inode table of NVRAM meta vol. has failed...");

        return false;
    }

    return true;
}

bool
MetaFileInodeManager::_StoreInodeToMedia(MetaStorageType media, MetaLpnType baseLpn)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& inodeInUseBitmap = inodeHdr->GetInodeInUseBitmap();
    uint32_t size = inodeInUseBitmap.size();
    for (uint32_t idx = 0; idx < size; idx++)
    {
        if (inodeInUseBitmap.test(idx))
        {
            if (false == inodeTable->Store(arrayName, media, baseLpn, idx, 1 /*LpnCnts*/))
            {
                return false;
            }
        }
    }

    return true;
}

void
MetaFileInodeManager::Finalize(void)
{
    fd2InodeMap.clear();
}

bool
MetaFileInodeManager::CreateFileInode(MetaFsFileControlRequest& reqMsg, FileDescriptorType newFd, MetaFilePageMap& pageMap, FileSizeType dataChunkSizeInMetaPage)
{
    MetaFileInode& newInode = _AllocNewInodeEntry(newFd);

    MetaFileInodeCreateReq inodeReq;
    inodeReq.Setup(reqMsg, newFd, mediaType, pageMap);
    newInode.BuildNewEntry(inodeReq, dataChunkSizeInMetaPage);

    if (true != SaveContent())
    {
        return false;
    }

    _UpdateFd2InodeMap(newFd, newInode);
    return true;
}

bool
MetaFileInodeManager::DeleteFileInode(FileDescriptorType& fd, std::string& arrayName)
{
    MetaFileInode& inode = GetFileInode(fd, arrayName);
    uint32_t entryIdx = inode.GetIndexInInodeTable();

    inode.SetInUse(false);
    inodeHdr->ClearInodeInUse(entryIdx);

    fd2InodeMap.erase(fd);
    fd2ArrayMap.erase(fd);

    if (true != SaveContent())
    {
        return false;
    }

    return true;
}

MetaFileInode&
MetaFileInodeManager::_AllocNewInodeEntry(FileDescriptorType& newFd)
{
    uint32_t entryIdx = inodeHdr->GetFreeInodeEntryIdx();
    inodeHdr->SetInodeInUse(entryIdx);
    MetaFileInode& freeInode = inodeTable->GetContent()->entries[entryIdx];
    freeInode.CleanupEntry();
    freeInode.SetIndexInInodeTable(entryIdx);

    return freeInode;
}

FileSizeType
MetaFileInodeManager::GetFileSize(const FileDescriptorType fd, std::string& arrayName)
{
    MetaFileInode& inode = GetFileInode(fd, arrayName);
    return inode.data.basic.field.fileByteSize;
}

FileSizeType
MetaFileInodeManager::GetDataChunkSize(const FileDescriptorType fd, std::string& arrayName)
{
    MetaFileInode& inode = GetFileInode(fd, arrayName);
    return inode.data.basic.field.dataChunkSize;
}

MetaLpnType
MetaFileInodeManager::GetFileBaseLpn(const FileDescriptorType fd, std::string& arrayName)
{
    MetaFileInode& inode = GetFileInode(fd, arrayName);
    return inode.data.basic.field.pagemap.baseMetaLpn;
}

MetaFileInode&
MetaFileInodeManager::GetFileInode(const FileDescriptorType fd, std::string& arrayName)
{
    auto item = fd2InodeMap.find(fd);
    assert(item != fd2InodeMap.end());
    return *item->second;
}

MetaFileInode&
MetaFileInodeManager::GetInodeEntry(const uint32_t entryIdx)
{
    return inodeTable->GetContent()->entries[entryIdx];
}

void
MetaFileInodeManager::_UpdateFd2InodeMap(FileDescriptorType fd, MetaFileInode& inode)
{
    fd2InodeMap.insert(std::make_pair(fd, &inode));
}

MetaFileExtent*
MetaFileInodeManager::GetInodeHdrExtentMapBase(void)
{
    return inodeHdr->GetFileExtentContentBase();
}

size_t
MetaFileInodeManager::GetInodeHdrExtentMapSize(void)
{
    return inodeHdr->GetFileExtentContentSize();
}

void
MetaFileInodeManager::RemoveFDsInUse(std::map<FileDescriptorType, FileDescriptorType>& freeFDMap)
{
    InodeInUseBitmap& src = inodeHdr->GetContent()->inodeInUseBitmap;

    int inodeEntryNum = src.bits.size();
    for (int idx = 0; idx < inodeEntryNum; idx++)
    {
        if (src.bits.test(idx))
        {
            FileDescriptorType fd = inodeTable->GetFileDescriptor(idx);
            assert(freeFDMap.find(fd) != freeFDMap.end());
            freeFDMap.erase(fd);
        }
    }
}

void
MetaFileInodeManager::PopulateFDMapWithVolumeType(std::unordered_map<FileDescriptorType, MetaVolumeType>& dest)
{
    InodeTableHeaderContent* content = inodeHdr->GetContent();
    InodeInUseBitmap& inodeInUseBitmap = content->inodeInUseBitmap;

    int inodeEntryNum = inodeInUseBitmap.bits.size();
    for (int idx = inodeEntryNum - 1; idx >= 0; idx--)
    {
        if (inodeInUseBitmap.bits.test(idx))
        {
            FileDescriptorType fd = inodeTable->GetFileDescriptor(idx);
            dest.insert(std::make_pair(fd, volumeType));

            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[PopulateFDMapWithVolumeType] fd2VolTypehMap] fd={} volumeType={}",
                fd, (int)volumeType);
        }
    }
}

void
MetaFileInodeManager::PopulateFileNameWithVolumeType(std::unordered_map<StringHashType, MetaVolumeType>& dest)
{
    // FIXME: need to polish
    InodeTableContent* content = inodeTable->GetContent();
    MetaInodeTableArray& inodes = content->entries;
    for (auto& inode : inodes) // FIXME: need to improve
    {
        if (inode.IsInUse())
        {
            StringHashType hashKey = MetaFileUtil::GetHashKeyFromFileName(inode.data.basic.field.fileName.ToString());
            dest.insert(std::make_pair(hashKey, volumeType));
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "fileKey2VolumeType populated: {}, {}",
                hashKey, (int)volumeType);
        }
    }
}

void
MetaFileInodeManager::PopulateFileKeyWithFD(std::unordered_map<StringHashType, FileDescriptorType>& dest)
{
    // FIXME: need to polish
    MetaInodeTableArray& inodes = inodeTable->GetContent()->entries;
    for (auto& inode : inodes) // FIXME: need to improve
    {
        if (inode.IsInUse())
        {
            StringHashType hashKey = MetaFileUtil::GetHashKeyFromFileName(inode.data.basic.field.fileName.ToString());
            dest.insert(std::make_pair(hashKey, inode.data.basic.field.fd));

            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "fileName={}, hashKey={}, fd={}",
                inode.data.basic.field.fileName.ToString(),
                hashKey, inode.data.basic.field.fd);
        }
    }
}

bool
MetaFileInodeManager::IsFileInodeInUse(const FileDescriptorType fd)
{
    return inodeHdr->IsFileInodeInUse(fd);
}

size_t
MetaFileInodeManager::GetTotalAllocatedInodeCnt(void)
{
    return inodeHdr->GetTotalAllocatedInodeCnt();
}

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaFileInodeManager::Compaction(void)
{
    bool ret = true;
    InodeTableHeaderContent* content = inodeHdr->GetContent();
    MetaLpnType maxLpn = 0;

    for (uint32_t index = 0; index < MetaFsConfig::MAX_VOLUME_CNT; index++)
    {
        if (content->allocExtentsList[index].GetCount() == 0)
        {
            break;
        }
        maxLpn = content->allocExtentsList[index].GetStartLpn() + content->allocExtentsList[index].GetCount();
    }

    RegionDeque* regionList = new RegionDeque(arrayName, mediaType, inodeTable->GetContent(), maxLpn);

    regionList->ReadRegions();
    regionList->InsertDone();
    if (regionList->IsCompactionRequired())
    {
        POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Compaction required");

        if (true == regionList->Compaction())
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "Compaction done");

            // update free extents
            regionList->UpdateExtentsList(content->allocExtentsList);
        }

        if (true != SaveContent())
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "Failed to save the result");
            ret = false;
        }
    }

    delete regionList;

    return ret;
}
#endif
} // namespace pos
