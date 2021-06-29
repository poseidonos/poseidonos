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

#include "mf_inode.h"
#include "inode_table_header.h"
#include "mf_inode_req.h"
#include "inode_table.h"
#include "mf_property.h"
#include "metafs_control_request.h"
#include "on_volume_meta_region_mgr.h"
#include "os_header.h"

namespace pos
{
// handle all operations required to meta file inode and its header
class MetaFileInodeManager : public OnVolumeMetaRegionManager
{
public:
    explicit MetaFileInodeManager(std::string arrayName);
    ~MetaFileInodeManager(void);

    virtual void Init(MetaVolumeType volType, MetaLpnType baseLpn, MetaLpnType maxLpn) override;
    virtual MetaLpnType GetRegionSizeInLpn(void) override;
    virtual void Bringup(void) override;
    virtual bool SaveContent(void) override;
    virtual void Finalize(void) override;
    virtual void SetMss(MetaStorageSubsystem* metaStorage);

    FileSizeType GetFileSize(const FileDescriptorType fd);
    FileSizeType GetDataChunkSize(const FileDescriptorType fd);
    MetaLpnType GetFileBaseLpn(const FileDescriptorType fd);

    void CreateInitialInodeContent(uint32_t maxInodeNum);
    bool LoadInodeContent(void);
    MetaLpnType GetRegionBaseLpn(MetaRegionType regionType);
    MetaLpnType GetRegionSizeInLpn(MetaRegionType regionType);
    void PopulateFDMapWithVolumeType(std::unordered_map<FileDescriptorType, MetaVolumeType>& dest);
    void PopulateFileNameWithVolumeType(std::unordered_map<StringHashType, MetaVolumeType>& dest);
    void PopulateFileKeyWithFD(std::unordered_map<StringHashType, FileDescriptorType>& dest);
    FileDescriptorType Alloc(std::string& fileName);

    bool CreateFileInode(MetaFsFileControlRequest& req, FileDescriptorType newFd, MetaFilePageMap& pageMap, FileSizeType dataChunkSizeInMetaPage);
    bool DeleteFileInode(FileDescriptorType& fd);
    bool IsFileInodeExist(std::string& fileName);
    MetaFileExtent* GetInodeHdrExtentMapBase(void);
    size_t GetInodeHdrExtentMapSize(void);
    void RemoveFDsInUse(std::map<FileDescriptorType, FileDescriptorType>& dstFreeFDMap);
    MetaFileInode& GetFileInode(const FileDescriptorType fd);
    MetaFileInode& GetInodeEntry(const uint32_t entryIdx);
    bool IsFileInodeInUse(const FileDescriptorType fd);
    size_t GetTotalAllocatedInodeCnt(void);

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(void);
#endif

    bool BackupContent(MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableLpnCnts);
    bool RestoreContent(MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableLpnCnts);

private:
    MetaFileInode& _AllocNewInodeEntry(FileDescriptorType& newFd);
    void _UpdateFd2InodeMap(FileDescriptorType fd, MetaFileInode& inode);
    void _BuildF2InodeMap(void);
    bool _LoadInodeFromMedia(MetaStorageType media, MetaLpnType baseLpn);
    bool _StoreInodeToMedia(MetaStorageType media, MetaLpnType baseLpn);

    InodeTableHeader* inodeHdr;
    InodeTable* inodeTable;
    std::unordered_map<FileDescriptorType, MetaFileInode*> fd2InodeMap;
    std::unordered_map<FileDescriptorType, std::string> fd2ArrayMap;
    MetaStorageSubsystem* metaStorage = nullptr;
};
} // namespace pos
