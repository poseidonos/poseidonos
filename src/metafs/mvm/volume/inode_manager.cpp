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
#include <vector>
#include "inode_manager.h"
#include "meta_file_util.h"
#include "metafs_log.h"

#define PRINT_INFO 0

namespace pos
{
InodeManager::InodeManager(int arrayId)
: OnVolumeMetaRegionManager(arrayId),
  inodeHdr(nullptr),
  inodeTable(nullptr),
  metaStorage(nullptr),
  fdAllocator(nullptr),
  extentAllocator(nullptr)
{
}

InodeManager::InodeManager(InodeTableHeader* inodeHdr, InodeTable* inodeTable,
        FileDescriptorAllocator* fdAllocator, ExtentAllocator* extentAllocator,
        int arrayId)
: InodeManager(arrayId)
{
    this->inodeHdr = inodeHdr;
    this->inodeTable = inodeTable;
    this->fdAllocator = fdAllocator;
    this->extentAllocator = extentAllocator;
}

InodeManager::~InodeManager(void)
{
    delete inodeHdr;
    delete inodeTable;
    delete fdAllocator;
    delete extentAllocator;
}

void
InodeManager::Init(MetaVolumeType volumeType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    OnVolumeMetaRegionManager::Init(volumeType, baseLpn, maxLpn);

    MetaLpnType targetBaseLpn = baseLpn;

    if (nullptr == inodeHdr)
    {
        inodeHdr = new InodeTableHeader(volumeType, targetBaseLpn);
    }
    targetBaseLpn += inodeHdr->GetLpnCntOfRegion();

    if (nullptr == inodeTable)
    {
        inodeTable = new InodeTable(volumeType, targetBaseLpn);
    }
    targetBaseLpn += inodeTable->GetLpnCntOfRegion();

    if (nullptr == fdAllocator)
    {
        fdAllocator = new FileDescriptorAllocator();
    }

    if (nullptr == extentAllocator)
    {
        extentAllocator = new ExtentAllocator();
    }
    extentAllocator->Init(targetBaseLpn, maxLpn);
}

MetaLpnType
InodeManager::GetRegionBaseLpn(MetaRegionType regionType)
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
InodeManager::GetRegionSizeInLpn(MetaRegionType regionType)
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
InodeManager::GetRegionSizeInLpn(void)
{
    MetaLpnType inodeContentSum = 0;
    inodeContentSum += inodeHdr->GetLpnCntOfRegion();
    inodeContentSum += inodeTable->GetLpnCntOfRegion();

    return inodeContentSum;
}

void
InodeManager::Bringup(void)
{
    inodeHdr->BuildFreeInodeEntryMap();
    _BuildF2InodeMap();
    _UpdateFdAllocator();
}

void
InodeManager::_BuildF2InodeMap(void)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& inodeInUseBitmap =
                            inodeHdr->GetInodeInUseBitmap();
    uint32_t size = inodeInUseBitmap.size();
    for (uint32_t idx = 0; idx < size; idx++)
    {
        if (inodeInUseBitmap.test(idx))
        {
            MetaFileInode& inode = inodeTable->GetInode(idx);
            _UpdateFd2InodeMap(inode.data.basic.field.fd, inode);
        }
    }
}

void
InodeManager::CreateInitialInodeContent(uint32_t maxInodeNum)
{
    inodeHdr->Create(maxInodeNum);
    inodeTable->Create(maxInodeNum);

    std::vector<pos::MetaFileExtent> extentList =
                            extentAllocator->GetAllocatedExtentList();
    inodeHdr->SetFileExtentContent(extentList);
}

bool
InodeManager::LoadInodeContent(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Load inode header content...");

    if (true != inodeHdr->Load())
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

    std::vector<pos::MetaFileExtent> extentList = inodeHdr->GetFileExtentContent();
    extentAllocator->SetAllocatedExtentList(extentList);

    return true;
}

/* Restore : meta in SSD volume to meta in NVRAM volume */
bool
InodeManager::RestoreContent(MetaVolumeType targetVol, MetaLpnType baseLpn,
    MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableCnts)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Restore inode header content] from vol= {}, baseLpn={}, LpnCnts={}...",
        targetVol, baseLpn, iNodeHdrLpnCnts);

    bool isSuccess;
    MetaStorageType media = MetaFileUtil::ConvertToMediaType(targetVol);

    isSuccess = inodeHdr->Load(media, baseLpn, 0 /*idx */, iNodeHdrLpnCnts);
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
InodeManager::_LoadInodeFromMedia(MetaStorageType media, MetaLpnType baseLpn)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& inodeInUseBitmap =
                                    inodeHdr->GetInodeInUseBitmap();
    uint32_t size = inodeInUseBitmap.size();
    for (uint32_t idx = 0; idx < size; idx++)
    {
        if (inodeInUseBitmap.test(idx))
        {
            if (false == inodeTable->Load(media, baseLpn,
                                idx * MetaFsConfig::LPN_COUNT_PER_INODE,
                                MetaFsConfig::LPN_COUNT_PER_INODE /*LpnCnts*/))
            {
                return false;
            }
        }
    }

    return true;
}

bool
InodeManager::SaveContent(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Save inode header content...");

    if (true != inodeHdr->Store())
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
InodeManager::BackupContent(MetaVolumeType targetVol, MetaLpnType baseLpn,
    MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableCnts)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Bakcup inode Header Content] to vol= {}, baseLpn={}, LpnCnts={}...",
        targetVol, baseLpn, iNodeHdrLpnCnts);

    bool isSuccess;
    MetaStorageType media = MetaFileUtil::ConvertToMediaType(targetVol);

    isSuccess = inodeHdr->Store(media, baseLpn, 0 /*buf idx */, iNodeHdrLpnCnts);
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
InodeManager::_StoreInodeToMedia(MetaStorageType media, MetaLpnType baseLpn)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& inodeInUseBitmap =
                                    inodeHdr->GetInodeInUseBitmap();
    uint32_t size = inodeInUseBitmap.size();
    for (uint32_t idx = 0; idx < size; idx++)
    {
        if (inodeInUseBitmap.test(idx))
        {
            if (false == inodeTable->Store(media, baseLpn,
                                idx * MetaFsConfig::LPN_COUNT_PER_INODE,
                                MetaFsConfig::LPN_COUNT_PER_INODE /*LpnCnts*/))
            {
                return false;
            }
        }
    }

    return true;
}

void
InodeManager::Finalize(void)
{
    fd2InodeMap.clear();
    fdAllocator->Reset();
}

void
InodeManager::SetMss(MetaStorageSubsystem* metaStorage)
{
    this->metaStorage = metaStorage;
    inodeHdr->SetMss(metaStorage);
    inodeTable->SetMss(metaStorage);
}

std::pair<FileDescriptorType, POS_EVENT_ID>
InodeManager::CreateFileInode(MetaFsFileControlRequest& reqMsg)
{
    MetaLpnType totalLpnCount = 0;
    FileDescriptorType fd = fdAllocator->Alloc(*reqMsg.fileName);
    MetaFileInode& newInode = _AllocNewInodeEntry(fd);

    FileSizeType userDataChunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    MetaLpnType requestLpnCnt = (reqMsg.fileByteSize + userDataChunkSize - 1) / userDataChunkSize;
    std::vector<MetaFileExtent> extents = extentAllocator->AllocExtents(requestLpnCnt);

#if (PRINT_INFO == 1)
    std::cout << "========= CreateFileInode: " << *reqMsg.fileName << std::endl;
    for (auto& extent : extents)
    {
        std::cout << "{" << extent.GetStartLpn() << ", ";
        std::cout << extent.GetStartLpn() + extent.GetCount() << "} ";
        std::cout << extent.GetCount() << std::endl;
        totalLpnCount += extent.GetCount();
    }
    std::cout << "lpn count: " << totalLpnCount << std::endl;
#endif

    MetaFileInodeCreateReq inodeReq;
    inodeReq.Setup(reqMsg, fd, mediaType, &extents);

    newInode.BuildNewEntry(inodeReq, MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE);

    _UpdateFd2InodeMap(fd, newInode);

    totalLpnCount = 0;
    for (auto& extent : extents)
    {
        POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "[Metadata File] Allocate an extent, startLpn={}, count={}",
            extent.GetStartLpn(), extent.GetCount());
            totalLpnCount += extent.GetCount();
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Metadata File] Create volType={}, fd={}, fileName={}, totalLpnCnt={}",
        (int)mediaType, fd, *reqMsg.fileName, totalLpnCount);

    std::vector<pos::MetaFileExtent> usedExtentsInVolume =
                                    extentAllocator->GetAllocatedExtentList();
    inodeHdr->SetFileExtentContent(usedExtentsInVolume);

    if (true != SaveContent())
    {
        for (auto& extent : extents)
        {
            extentAllocator->AddToFreeList(extent.GetStartLpn(), extent.GetCount());

            POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[Metadata File] Release an extent, startLpn={}, count={}",
                extent.GetStartLpn(), extent.GetCount());
        }

        return std::make_pair(0, POS_EVENT_ID::MFS_META_SAVE_FAILED);
    }

    return std::make_pair(fd, POS_EVENT_ID::SUCCESS);
}

std::pair<FileDescriptorType, POS_EVENT_ID>
InodeManager::DeleteFileInode(MetaFsFileControlRequest& reqMsg)
{
    MetaLpnType totalLpnCount = 0;
    std::vector<MetaFileExtent> extents;
    FileDescriptorType fd = LookupDescriptorByName(*reqMsg.fileName);
    MetaLpnType count = GetExtent(fd, extents);
    assert(count > 0);

#if (PRINT_INFO == 1)
    std::cout << "========= DeleteFileInode: " << *reqMsg.fileName << std::endl;
    MetaLpnType totalLpnCount = 0;
    for (auto& extent : extents)
    {
        std::cout << "{" << extent.GetStartLpn() << ", ";
        std::cout << extent.GetStartLpn() + extent.GetCount() << "} ";
        std::cout << extent.GetCount() << std::endl;
        totalLpnCount += extent.GetCount();
    }
    std::cout << "lpn count: " << totalLpnCount << std::endl;
#endif

    MetaFileInode& inode = GetFileInode(fd);
    uint32_t entryIdx = inode.GetIndexInInodeTable();

    inode.SetInUse(false);
    inodeHdr->ClearInodeInUse(entryIdx);

    fd2InodeMap.erase(fd);

    totalLpnCount = 0;
    for (auto& extent : extents)
    {
        extentAllocator->AddToFreeList(extent.GetStartLpn(), extent.GetCount());

        POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Metadata File] Release an extent, startLpn={}, count={}",
        extent.GetStartLpn(), extent.GetCount());
        totalLpnCount += extent.GetCount();
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Metadata File] Delete volType={}, fd={}, fileName={}, totalLpnCnt={}",
        (int)volumeType, fd, *reqMsg.fileName, totalLpnCount);

    fdAllocator->Free(*reqMsg.fileName, fd);

    extentAllocator->PrintFreeExtentsList();

    std::vector<pos::MetaFileExtent> usedExtentsInVolume =
                                    extentAllocator->GetAllocatedExtentList();
    inodeHdr->SetFileExtentContent(usedExtentsInVolume);

    if (true != SaveContent())
    {
        return std::make_pair(0, POS_EVENT_ID::MFS_META_SAVE_FAILED);
    }

    return std::make_pair(fd, POS_EVENT_ID::SUCCESS);
}

uint32_t
InodeManager::GetUtilizationInPercent(void)
{
    return ((maxLpn - extentAllocator->GetAvailableLpnCount()) * 100) / maxLpn;
}

size_t
InodeManager::GetAvailableSpace(void)
{
    MetaLpnType maxSize = extentAllocator->GetAvailableSpace();

    maxSize -= maxSize % MetaFsConfig::LPN_COUNT_PER_EXTENT;

    return maxSize * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
}

bool
InodeManager::CheckFileInActive(FileDescriptorType fd)
{
    if (activeFiles.find(fd) == activeFiles.end())
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "File descriptor={} is not found in active fd list", fd);
        return false;
    }
    return true;
}

POS_EVENT_ID
InodeManager::AddFileInActiveList(FileDescriptorType fd)
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

bool
InodeManager::IsGivenFileCreated(StringHashType fileKey)
{
    return fdAllocator->IsGivenFileCreated(fileKey);
}

void
InodeManager::RemoveFileFromActiveList(FileDescriptorType fd)
{
    assert(activeFiles.find(fd) != activeFiles.end());
    activeFiles.erase(fd);
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "File descriptor={} is removed from active fd list, remained activeFiles={}",
        fd, activeFiles.size());
}

size_t
InodeManager::GetFileCountInActive(void)
{
    return activeFiles.size();
}

MetaLpnType
InodeManager::GetMetaFileBaseLpn(void)
{
    return extentAllocator->GetFileBaseLpn();
}

void
InodeManager::SetMetaFileBaseLpn(MetaLpnType lpn)
{
    extentAllocator->SetFileBaseLpn(lpn);
}

FileDescriptorType
InodeManager::LookupDescriptorByName(std::string& fileName)
{
    return fdAllocator->FindFdByName(fileName);
}

std::string
InodeManager::LookupNameByDescriptor(FileDescriptorType fd)
{
    auto item = fd2InodeMap.find(fd);
    if (item == fd2InodeMap.end())
        return "";

    return item->second->data.basic.field.fileName.ToString();
}

MetaFileInode&
InodeManager::_AllocNewInodeEntry(FileDescriptorType& newFd)
{
    uint32_t entryIdx = inodeHdr->GetFreeInodeEntryIdx();
    inodeHdr->SetInodeInUse(entryIdx);
    MetaFileInode& freeInode = inodeTable->GetInode(entryIdx);
    freeInode.CleanupEntry();
    freeInode.SetIndexInInodeTable(entryIdx);

    return freeInode;
}

FileSizeType
InodeManager::GetFileSize(const FileDescriptorType fd)
{
    MetaFileInode& inode = GetFileInode(fd);
    return inode.data.basic.field.fileByteSize;
}

FileSizeType
InodeManager::GetDataChunkSize(const FileDescriptorType fd)
{
    MetaFileInode& inode = GetFileInode(fd);
    return inode.data.basic.field.dataChunkSize;
}

MetaLpnType
InodeManager::GetFileBaseLpn(const FileDescriptorType fd)
{
    MetaFileInode& inode = GetFileInode(fd);
    return inode.data.basic.field.pagemap[0].GetStartLpn();
}

uint32_t
InodeManager::GetExtent(const FileDescriptorType fd,
                        std::vector<MetaFileExtent>& extents /* output */)
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
InodeManager::GetFileInode(const FileDescriptorType fd)
{
    auto item = fd2InodeMap.find(fd);
    assert(item != fd2InodeMap.end());
    return *item->second;
}

MetaFileInode&
InodeManager::GetInodeEntry(const uint32_t entryIdx)
{
    return inodeTable->GetInode(entryIdx);
}

void
InodeManager::_UpdateFd2InodeMap(FileDescriptorType fd, MetaFileInode& inode)
{
    fd2InodeMap.insert(std::make_pair(fd, &inode));
}

void
InodeManager::_UpdateFdAllocator(void)
{
    // update freeFdMap
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& src =
                                        inodeHdr->GetInodeInUseBitmap();

    int inodeEntryNum = src.size();
    for (int idx = 0; idx < inodeEntryNum; idx++)
    {
        if (src.test(idx))
        {
            FileDescriptorType fd = inodeTable->GetFileDescriptor(idx);
            fdAllocator->UpdateFreeMap(fd);
        }
    }

    // update fileKey2FdLookupMap
    MetaFileInodeArray& inodes = inodeTable->GetInodeArray();
    for (auto& inode : inodes) // FIXME: need to improve
    {
        if (inode.IsInUse())
        {
            std::string fileName = inode.data.basic.field.fileName.ToString();
            StringHashType hashKey = MetaFileUtil::GetHashKeyFromFileName(fileName);

            fdAllocator->UpdateLookupMap(hashKey, inode.data.basic.field.fd);

            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "fileName={}, hashKey={}, fd={}",
                inode.data.basic.field.fileName.ToString(),
                hashKey, inode.data.basic.field.fd);
        }
    }
}

void
InodeManager::PopulateFDMapWithVolumeType(std::unordered_map<FileDescriptorType, MetaVolumeType>& dest)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& src =
                                        inodeHdr->GetInodeInUseBitmap();

    int inodeEntryNum = src.size();
    for (int idx = inodeEntryNum - 1; idx >= 0; idx--)
    {
        if (src.test(idx))
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
InodeManager::PopulateFileNameWithVolumeType(std::unordered_map<StringHashType, MetaVolumeType>& dest)
{
    // FIXME: need to polish
    MetaFileInodeArray& inodes = inodeTable->GetInodeArray();

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

bool
InodeManager::IsFileInodeInUse(const FileDescriptorType fd)
{
    return inodeHdr->IsFileInodeInUse(fd);
}

size_t
InodeManager::GetTotalAllocatedInodeCnt(void)
{
    return inodeHdr->GetTotalAllocatedInodeCnt();
}

MetaLpnType
InodeManager::GetTheLastValidLpn(void)
{
    std::vector<MetaFileExtent> extents = inodeHdr->GetFileExtentContent();

    if (extents.size() == 0)
        return 0;

    MetaLpnType lpn = extents[extents.size() - 1].GetStartLpn();
    lpn += extents[extents.size() - 1].GetCount() - 1;

    return lpn;
}
} // namespace pos
