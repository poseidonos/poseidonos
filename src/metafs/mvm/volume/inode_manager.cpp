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

#include "inode_manager.h"

#include <string>
#include <vector>

#include "src/metafs/common/meta_file_util.h"
#include "src/metafs/log/metafs_log.h"

namespace pos
{
InodeManager::InodeManager(const int arrayId, InodeTableHeader* inodeHdr,
    InodeTable* inodeTable, FileDescriptorAllocator* fdAllocator,
    ExtentAllocator* extentAllocator)
: OnVolumeMetaRegionManager(arrayId),
  inodeHdr_(inodeHdr),
  inodeTable_(inodeTable),
  metaStorage_(nullptr),
  fdAllocator_(fdAllocator),
  extentAllocator_(extentAllocator)
{
}

InodeManager::~InodeManager(void)
{
    if (inodeHdr_)
    {
        delete inodeHdr_;
        inodeHdr_ = nullptr;
    }

    if (inodeTable_)
    {
        delete inodeTable_;
        inodeTable_ = nullptr;
    }

    if (fdAllocator_)
    {
        delete fdAllocator_;
        fdAllocator_ = nullptr;
    }

    if (extentAllocator_)
    {
        delete extentAllocator_;
        extentAllocator_ = nullptr;
    }
}

void
InodeManager::Init(MetaVolumeType volumeType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    OnVolumeMetaRegionManager::Init(volumeType, baseLpn, maxLpn);
    MetaLpnType targetBaseLpn = baseLpn;

    if (!inodeHdr_)
        inodeHdr_ = new InodeTableHeader(volumeType, targetBaseLpn);
    targetBaseLpn += inodeHdr_->GetLpnCntOfRegion();

    if (!inodeTable_)
        inodeTable_ = new InodeTable(volumeType, targetBaseLpn);
    targetBaseLpn += inodeTable_->GetLpnCntOfRegion();

    if (!fdAllocator_)
        fdAllocator_ = new FileDescriptorAllocator();

    if (!extentAllocator_)
        extentAllocator_ = new ExtentAllocator();
    extentAllocator_->Init(targetBaseLpn, maxLpn);
}

MetaLpnType
InodeManager::GetRegionBaseLpn(const MetaRegionType regionType) const
{
    switch (regionType)
    {
        case MetaRegionType::FileInodeHdr:
            return inodeHdr_->GetBaseLpn();

        case MetaRegionType::FileInodeTable:
            return inodeTable_->GetBaseLpn();

        default:
            POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
                "The region type is not valid, regionType: {}",
                (int)regionType);
            assert(false);
    }
}

MetaLpnType
InodeManager::GetRegionSizeInLpn(MetaRegionType regionType) const
{
    switch (regionType)
    {
        case MetaRegionType::FileInodeHdr:
            return inodeHdr_->GetLpnCntOfRegion();

        case MetaRegionType::FileInodeTable:
            return inodeTable_->GetLpnCntOfRegion();

        default:
            POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
                "The region type is not valid, regionType: {}",
                (int)regionType);
            assert(false);
    }
}

MetaLpnType
InodeManager::GetRegionSizeInLpn(void)
{
    return inodeHdr_->GetLpnCntOfRegion() + inodeTable_->GetLpnCntOfRegion();
}

void
InodeManager::Bringup(void)
{
    inodeHdr_->BuildFreeInodeEntryMap();
    _BuildF2InodeMap();
    _UpdateFdAllocator();
}

void
InodeManager::_BuildF2InodeMap(void)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& inodeInUseBitmap =
        inodeHdr_->GetInodeInUseBitmap();

    for (uint32_t idx = 0; idx < inodeInUseBitmap.size(); idx++)
    {
        if (inodeInUseBitmap.test(idx))
        {
            MetaFileInode& inode = inodeTable_->GetInode(idx);
            fd2InodeMap_.insert({inode.data.basic.field.fd, &inode});
        }
    }
}

void
InodeManager::CreateInitialInodeContent(const uint32_t maxInodeNum) const
{
    inodeHdr_->Create(maxInodeNum);
    inodeTable_->Create(maxInodeNum);

    std::vector<pos::MetaFileExtent> extentList = extentAllocator_->GetAllocatedExtentList();
    inodeHdr_->SetFileExtentContent(extentList);
}

bool
InodeManager::LoadContent(void)
{
    if (!inodeHdr_->Load())
    {
        POS_TRACE_ERROR(EID(MFS_META_LOAD_FAILED),
            "Load I/O for MFS inode header has failed...");
        return false;
    }

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "Total allocated file descriptor count: {}",
        inodeHdr_->GetTotalAllocatedInodeCnt());

    MetaLpnType startBaseLpn = GetRegionBaseLpn(MetaRegionType::FileInodeTable);

    if (!_LoadInodeFromMedia(MetaFileUtil::ConvertToMediaType(volumeType), startBaseLpn))
    {
        POS_TRACE_ERROR(EID(MFS_META_LOAD_FAILED),
            "Load I/O for MFS inode content has failed...");
        return false;
    }

    std::vector<pos::MetaFileExtent> extentList = inodeHdr_->GetFileExtentContent();
    extentAllocator_->SetAllocatedExtentList(extentList);

    return true;
}

/* Restore : meta in SSD volume to meta in NVRAM volume */
bool
InodeManager::RestoreContent(MetaVolumeType targetVol, MetaLpnType baseLpn,
    MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableCnts)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "[Restore inode header content] from vol= {}, baseLpn={}, LpnCnts={}...",
        targetVol, baseLpn, iNodeHdrLpnCnts);

    MetaStorageType media = MetaFileUtil::ConvertToMediaType(targetVol);

    if (!inodeHdr_->Load(media, baseLpn, 0 /*idx */, iNodeHdrLpnCnts))
    {
        POS_TRACE_ERROR(EID(MFS_META_LOAD_FAILED),
            "Restore I/O for MFS inode header of NVRAM meta vol. has failed...");
        return false;
    }

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "[Restore the valid inode table content] from baseLpn={}, LpnCnts={}...",
        baseLpn + iNodeHdrLpnCnts, iNodeTableCnts);

    if (!_LoadInodeFromMedia(media, baseLpn + iNodeHdrLpnCnts))
    {
        POS_TRACE_ERROR(EID(MFS_META_LOAD_FAILED),
            "Restore I/O for MFS inode table of NVRAM meta vol. has failed...");

        return false;
    }

    return true;
}

bool
InodeManager::_LoadInodeFromMedia(const MetaStorageType media, const MetaLpnType baseLpn)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& inodeInUseBitmap =
        inodeHdr_->GetInodeInUseBitmap();

    for (uint32_t idx = 0; idx < inodeInUseBitmap.size(); idx++)
    {
        if (inodeInUseBitmap.test(idx) &&
            !inodeTable_->Load(media, baseLpn,
                idx * MetaFsConfig::LPN_COUNT_PER_INODE, MetaFsConfig::LPN_COUNT_PER_INODE))
        {
            return false;
        }
    }

    return true;
}

bool
InodeManager::SaveContent(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Save inode header content...");

    if (!inodeHdr_->Store())
    {
        POS_TRACE_ERROR(EID(MFS_META_SAVE_FAILED),
            "Store I/O for MFS inode header has failed...");

        return false;
    }

    MetaLpnType startBaseLpn = GetRegionBaseLpn(MetaRegionType::FileInodeTable);

    if (!_StoreInodeToMedia(MetaFileUtil::ConvertToMediaType(volumeType), startBaseLpn))
    {
        POS_TRACE_ERROR(EID(MFS_META_SAVE_FAILED),
            "Store I/O for MFS inode table has failed...");

        return false;
    }

    return true;
}

/* Backup : meta in NVRAM volume to meta in SSD volume) */
bool
InodeManager::BackupContent(MetaVolumeType targetVol, MetaLpnType baseLpn,
    MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableCnts)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "[Bakcup inode Header Content] to vol= {}, baseLpn={}, LpnCnts={}...",
        targetVol, baseLpn, iNodeHdrLpnCnts);

    MetaStorageType media = MetaFileUtil::ConvertToMediaType(targetVol);

    if (!inodeHdr_->Store(media, baseLpn, 0 /*buf idx */, iNodeHdrLpnCnts))
    {
        POS_TRACE_ERROR(EID(MFS_META_SAVE_FAILED),
            "Backup I/O for MFS inode header of NVRAM meta vol. has failed...");
        return false;
    }

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "[Backup the valid inode Table Content] to Backup vol= {}, baseLpn={}, LpnCnts={}...",
        targetVol, baseLpn + iNodeHdrLpnCnts, iNodeHdrLpnCnts);

    MetaLpnType startBaseLpn = baseLpn + iNodeHdrLpnCnts;

    if (!_StoreInodeToMedia(media, startBaseLpn))
    {
        POS_TRACE_ERROR(EID(MFS_META_SAVE_FAILED),
            "Backup I/O for MFS inode table of NVRAM meta vol. has failed...");

        return false;
    }

    return true;
}

bool
InodeManager::_StoreInodeToMedia(const MetaStorageType media, const MetaLpnType baseLpn)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& inodeInUseBitmap = inodeHdr_->GetInodeInUseBitmap();

    for (uint32_t idx = 0; idx < inodeInUseBitmap.size(); idx++)
    {
        if (inodeInUseBitmap.test(idx) && !inodeTable_->Store(media, baseLpn, idx * MetaFsConfig::LPN_COUNT_PER_INODE, MetaFsConfig::LPN_COUNT_PER_INODE))
        {
            return false;
        }
    }

    return true;
}

void
InodeManager::Finalize(void)
{
    fd2InodeMap_.clear();
    fdAllocator_->Reset();
}

void
InodeManager::SetMss(MetaStorageSubsystem* metaStorage)
{
    metaStorage_ = metaStorage;
    inodeHdr_->SetMss(metaStorage);
    inodeTable_->SetMss(metaStorage);
}

MetaLpnType
InodeManager::GetAvailableLpnCount(void) const
{
    return extentAllocator_->GetAvailableLpnCount();
}

size_t
InodeManager::GetAvailableSpace(void) const
{
    MetaLpnType maxSize = extentAllocator_->GetAvailableLpnCount();

    maxSize -= maxSize % MetaFsConfig::LPN_COUNT_PER_EXTENT;

    return maxSize * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
}

bool
InodeManager::CheckFileInActive(const FileDescriptorType fd) const
{
    if (activeFiles_.find(fd) == activeFiles_.end())
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "File descriptor {} is not found in active fd list", fd);
        return false;
    }
    return true;
}

POS_EVENT_ID
InodeManager::AddFileInActiveList(const FileDescriptorType fd)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    if (activeFiles_.find(fd) != activeFiles_.end())
    {
        rc = EID(MFS_FILE_OPEN_REPETITIONARY);
        POS_TRACE_ERROR((int)rc,
            "You attempt to open fd {} file twice. It is not allowed", fd);
    }
    else
    {
        activeFiles_.insert(fd);
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "File descriptor {} is added in active fd list", fd);
    }

    return rc;
}

void
InodeManager::RemoveFileFromActiveList(const FileDescriptorType fd)
{
    if (activeFiles_.find(fd) == activeFiles_.end())
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "File descriptor {} is not active file.", fd);
        assert(false);
    }

    activeFiles_.erase(fd);
    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "File descriptor {} is removed from active fd list, remained active file count: {}",
        fd, activeFiles_.size());
}

size_t
InodeManager::GetFileCountInActive(void) const
{
    return activeFiles_.size();
}

std::string
InodeManager::LookupNameByDescriptor(const FileDescriptorType fd) const
{
    auto item = fd2InodeMap_.find(fd);
    if (item == fd2InodeMap_.end())
        return "";

    return item->second->data.basic.field.fileName.ToString();
}

FileSizeType
InodeManager::GetFileSize(const FileDescriptorType fd) const
{
    return GetFileInode(fd).data.basic.field.fileByteSize;
}

FileSizeType
InodeManager::GetDataChunkSize(const FileDescriptorType fd) const
{
    return GetFileInode(fd).data.basic.field.dataChunkSize;
}

MetaLpnType
InodeManager::GetFileBaseLpn(const FileDescriptorType fd) const
{
    return GetFileInode(fd).data.basic.field.pagemap[0].GetStartLpn();
}

uint32_t
InodeManager::GetExtent(const FileDescriptorType fd, std::vector<MetaFileExtent>& extents /* output */) const
{
    MetaFileInode& inode = GetFileInode(fd);
    uint32_t extentCnt = inode.data.basic.field.pagemapCnt;

    for (uint32_t i = 0; i < extentCnt; ++i)
    {
        extents.push_back(inode.data.basic.field.pagemap[i]);
    }

    return extentCnt;
}

MetaFileInode&
InodeManager::GetFileInode(const FileDescriptorType fd) const
{
    auto item = fd2InodeMap_.find(fd);
    if (item == fd2InodeMap_.end())
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "File descriptor {} is not existed.", fd);
        assert(false);
    }
    return *item->second;
}

void
InodeManager::_UpdateFdAllocator(void)
{
    // update freeFdMap
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& src = inodeHdr_->GetInodeInUseBitmap();

    for (size_t idx = 0; idx < src.size(); idx++)
    {
        if (src.test(idx))
        {
            fdAllocator_->UpdateFreeMap(inodeTable_->GetFileDescriptor(idx));
        }
    }

    // update fileKey2FdLookupMap
    for (auto& inode : inodeTable_->GetInodeArray())
    {
        if (inode.IsInUse())
        {
            std::string fileName = inode.data.basic.field.fileName.ToString();
            StringHashType hashKey = MetaFileUtil::GetHashKeyFromFileName(fileName);

            fdAllocator_->UpdateLookupMap(hashKey, inode.data.basic.field.fd);

            MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
                "fileName={}, hashKey={}, fd={}",
                fileName, hashKey, inode.data.basic.field.fd);
        }
    }
}

void
InodeManager::PopulateFDMapWithVolumeType(std::unordered_map<FileDescriptorType, MetaVolumeType>& dest) const
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& src = inodeHdr_->GetInodeInUseBitmap();
    int inodeEntryNum = src.size();

    for (int idx = inodeEntryNum - 1; idx >= 0; idx--)
    {
        if (src.test(idx))
        {
            FileDescriptorType fd = inodeTable_->GetFileDescriptor(idx);
            dest.insert({fd, volumeType});

            MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
                "[PopulateFDMapWithVolumeType] fd2VolTypehMap] fd={} volumeType={}",
                fd, (int)volumeType);
        }
    }
}

void
InodeManager::PopulateFileNameWithVolumeType(std::unordered_map<StringHashType, MetaVolumeType>& dest) const
{
    for (auto& inode : inodeTable_->GetInodeArray())
    {
        if (inode.IsInUse())
        {
            StringHashType hashKey = MetaFileUtil::GetHashKeyFromFileName(inode.data.basic.field.fileName.ToString());
            dest.insert({hashKey, volumeType});
            MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
                "fileKey2VolumeType populated: {}, {}", hashKey, (int)volumeType);
        }
    }
}

MetaLpnType
InodeManager::GetTheLastValidLpn(void) const
{
    std::vector<MetaFileExtent> extents = inodeHdr_->GetFileExtentContent();

    if (!extents.size())
        return 0;

    MetaLpnType lpn = extents[extents.size() - 1].GetStartLpn();
    lpn += extents[extents.size() - 1].GetCount() - 1;

    return lpn;
}
} // namespace pos
