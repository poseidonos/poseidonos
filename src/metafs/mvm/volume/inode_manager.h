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

#pragma once

#include <map>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <unordered_set>

#include "mf_inode.h"
#include "inode_table_header.h"
#include "mf_inode_req.h"
#include "inode_table.h"
#include "mf_property.h"
#include "metafs_control_request.h"
#include "on_volume_meta_region_mgr.h"
#include "os_header.h"
#include "file_descriptor_allocator.h"
#include "extent_allocator.h"

namespace pos
{
using FileDescriptorInVolume = std::unordered_map<FileDescriptorType, MetaVolumeType>;
using FileHashInVolume = std::unordered_map<StringHashType, MetaVolumeType>;

// handle all operations required to meta file inode and its header
class InodeManager : public OnVolumeMetaRegionManager
{
public:
    explicit InodeManager(int arrayId);
    InodeManager(InodeTableHeader* inodeHdr, InodeTable* inodeTable,
        FileDescriptorAllocator* fdAllocator, ExtentAllocator* extentAllocator,
        int arrayId);
    virtual ~InodeManager(void);

    virtual void Init(MetaVolumeType volType, MetaLpnType baseLpn, MetaLpnType maxLpn) override;
    virtual MetaLpnType GetRegionSizeInLpn(void) override;
    virtual void Bringup(void) override;
    virtual bool SaveContent(void) override;
    virtual void Finalize(void) override;
    virtual void SetMss(MetaStorageSubsystem* metaStorage);

    virtual FileSizeType GetFileSize(const FileDescriptorType fd);
    virtual FileSizeType GetDataChunkSize(const FileDescriptorType fd);
    virtual MetaLpnType GetFileBaseLpn(const FileDescriptorType fd);
    virtual uint32_t GetExtent(const FileDescriptorType fd,
                std::vector<MetaFileExtent>& extents /* output */);

    virtual void CreateInitialInodeContent(uint32_t maxInodeNum);
    virtual bool LoadInodeContent(void);
    virtual MetaLpnType GetRegionBaseLpn(MetaRegionType regionType);
    virtual MetaLpnType GetRegionSizeInLpn(MetaRegionType regionType);
    virtual void PopulateFDMapWithVolumeType(FileDescriptorInVolume& dest);
    virtual void PopulateFileNameWithVolumeType(FileHashInVolume& dest);

    virtual std::pair<FileDescriptorType, POS_EVENT_ID> CreateFileInode(
                MetaFsFileControlRequest& reqMsg);
    virtual std::pair<FileDescriptorType, POS_EVENT_ID> DeleteFileInode(
                MetaFsFileControlRequest& reqMsg);
    virtual uint32_t GetUtilizationInPercent(void);
    virtual size_t GetAvailableSpace(void);

    virtual bool CheckFileInActive(FileDescriptorType fd);
    virtual POS_EVENT_ID AddFileInActiveList(FileDescriptorType fd);
    virtual void RemoveFileFromActiveList(FileDescriptorType fd);
    virtual size_t GetFileCountInActive(void);

    virtual void SetMetaFileBaseLpn(MetaLpnType lpn);
    virtual MetaLpnType GetMetaFileBaseLpn(void);

    virtual bool IsGivenFileCreated(StringHashType fileKey);
    virtual FileDescriptorType LookupDescriptorByName(std::string& fileName);
    virtual std::string LookupNameByDescriptor(FileDescriptorType fd);

    virtual MetaFileInode& GetFileInode(const FileDescriptorType fd);
    virtual MetaFileInode& GetInodeEntry(const uint32_t entryIdx);
    virtual bool IsFileInodeInUse(const FileDescriptorType fd);
    virtual size_t GetTotalAllocatedInodeCnt(void);
    virtual MetaLpnType GetTheLastValidLpn(void);

    virtual bool BackupContent(MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableLpnCnts);
    virtual bool RestoreContent(MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableLpnCnts);

    // only for test
    std::unordered_map<FileDescriptorType, MetaFileInode*>& GetInodeMap(void)
    {
        return fd2InodeMap;
    }

    // only for test
    std::unordered_set<FileDescriptorType>& GetActiveFiles(void)
    {
        return activeFiles;
    }

private:
    MetaFileInode& _AllocNewInodeEntry(FileDescriptorType& newFd);
    void _UpdateFd2InodeMap(FileDescriptorType fd, MetaFileInode& inode);
    void _BuildF2InodeMap(void);
    bool _LoadInodeFromMedia(MetaStorageType media, MetaLpnType baseLpn);
    bool _StoreInodeToMedia(MetaStorageType media, MetaLpnType baseLpn);
    void _UpdateFdAllocator(void);

    InodeTableHeader* inodeHdr;
    InodeTable* inodeTable;
    std::unordered_map<FileDescriptorType, MetaFileInode*> fd2InodeMap;
    std::unordered_set<FileDescriptorType> activeFiles;
    MetaStorageSubsystem* metaStorage;
    FileDescriptorAllocator* fdAllocator;
    ExtentAllocator* extentAllocator;
};
} // namespace pos
