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

#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "extent_allocator.h"
#include "file_descriptor_allocator.h"
#include "inode_table.h"
#include "inode_table_header.h"
#include "metafs_control_request.h"
#include "mf_inode.h"
#include "mf_inode_req.h"
#include "mf_property.h"
#include "on_volume_meta_region_mgr.h"
#include "os_header.h"
#include "src/metafs/mvm/volume/inode_creator.h"
#include "src/metafs/mvm/volume/inode_deleter.h"

namespace pos
{
using FileDescriptorInVolume = std::unordered_map<FileDescriptorType, MetaVolumeType>;
using FileHashInVolume = std::unordered_map<StringHashType, MetaVolumeType>;

// handle all operations required to meta file inode and its header
class InodeManager : public OnVolumeMetaRegionManager
{
public:
    InodeManager(void) = delete;
    InodeManager(const int arrayId, InodeTableHeader* inodeHdr_ = nullptr,
        InodeTable* inodeTable_ = nullptr, FileDescriptorAllocator* fdAllocator_ = nullptr,
        ExtentAllocator* extentAllocator_ = nullptr);
    virtual ~InodeManager(void);

    virtual void Init(MetaVolumeType volType, MetaLpnType baseLpn, MetaLpnType maxLpn) override;
    virtual MetaLpnType GetRegionSizeInLpn(void) override;
    virtual void Bringup(void) override;
    virtual bool SaveContent(void) override;
    virtual void Finalize(void) override;
    virtual void SetMss(MetaStorageSubsystem* metaStorage) override;

    virtual FileSizeType GetFileSize(const FileDescriptorType fd) const;
    virtual FileSizeType GetDataChunkSize(const FileDescriptorType fd) const;
    virtual MetaLpnType GetFileBaseLpn(const FileDescriptorType fd) const;
    virtual uint32_t GetExtent(const FileDescriptorType fd,
        std::vector<MetaFileExtent>& extents /* output */) const;

    virtual void CreateInitialInodeContent(const uint32_t maxInodeNum) const;
    virtual bool LoadContent(void);
    virtual MetaLpnType GetRegionBaseLpn(const MetaRegionType regionType) const;
    virtual MetaLpnType GetRegionSizeInLpn(const MetaRegionType regionType) const;
    virtual void PopulateFDMapWithVolumeType(FileDescriptorInVolume& dest) const;
    virtual void PopulateFileNameWithVolumeType(FileHashInVolume& dest) const;

    virtual MetaLpnType GetAvailableLpnCount(void) const;
    virtual size_t GetAvailableSpace(void) const;

    virtual bool CheckFileInActive(const FileDescriptorType fd) const;
    virtual POS_EVENT_ID AddFileInActiveList(const FileDescriptorType fd);
    virtual void RemoveFileFromActiveList(const FileDescriptorType fd);
    virtual size_t GetFileCountInActive(void) const;

    virtual std::string LookupNameByDescriptor(const FileDescriptorType fd) const;
    virtual MetaFileInode& GetFileInode(const FileDescriptorType fd) const;
    virtual MetaLpnType GetTheLastValidLpn(void) const;
    virtual void SetMetaFileBaseLpn(const MetaLpnType lpn) const
    {
        extentAllocator_->SetFileBaseLpn(lpn);
    }
    virtual MetaLpnType GetMetaFileBaseLpn(void) const
    {
        return extentAllocator_->GetFileBaseLpn();
    }
    virtual bool IsGivenFileCreated(const StringHashType fileKey) const
    {
        return fdAllocator_->IsGivenFileCreated(fileKey);
    }
    virtual FileDescriptorType LookupDescriptorByName(const std::string& fileName) const
    {
        return fdAllocator_->FindFdByName(fileName);
    }
    virtual MetaFileInode& GetInodeEntry(const uint32_t entryIdx) const
    {
        return inodeTable_->GetInode(entryIdx);
    }
    virtual bool IsFileInodeInUse(const FileDescriptorType fd) const
    {
        return inodeHdr_->IsFileInodeInUse(fd);
    }

    virtual bool BackupContent(MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableLpnCnts);
    virtual bool RestoreContent(MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableLpnCnts);

    /* only for test */
    std::unordered_map<FileDescriptorType, MetaFileInode*>& GetInodeMap(void)
    {
        return fd2InodeMap_;
    }
    std::unordered_set<FileDescriptorType>& GetActiveFiles(void)
    {
        return activeFiles_;
    }
    /* only for test */

private:
    void _BuildF2InodeMap(void);
    bool _LoadInodeFromMedia(const MetaStorageType media, const MetaLpnType baseLpn);
    bool _StoreInodeToMedia(const MetaStorageType media, const MetaLpnType baseLpn);
    void _UpdateFdAllocator(void);

    InodeTableHeader* inodeHdr_;
    InodeTable* inodeTable_;
    std::unordered_map<FileDescriptorType, MetaFileInode*> fd2InodeMap_;
    std::unordered_set<FileDescriptorType> activeFiles_;
    MetaStorageSubsystem* metaStorage_;
    FileDescriptorAllocator* fdAllocator_;
    ExtentAllocator* extentAllocator_;

    friend InodeCreator;
    friend InodeDeleter;
};
} // namespace pos
