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
#include "mf_inode_hdr.h"
#include "mf_inode_req.h"
#include "mf_inode_table.h"
#include "mfs_file_property.h"
#include "mvm_req.h"
#include "on_volume_meta_region_mgr.h"
#include "os_header.h"

// handle all operations required to meta file inode and its header
class MetaFileInodeMgrClass : public OnVolumeMetaRegionMgr
{
public:
    MetaFileInodeMgrClass(void);
    ~MetaFileInodeMgrClass(void);

    virtual void Init(MetaVolumeType volType, MetaLpnType baseLpn, MetaLpnType maxLpn) override;
    virtual MetaLpnType GetRegionSizeInLpn(void) override;
    virtual void Bringup(void) override;
    virtual bool SaveContent(void) override;
    virtual void Finalize(void) override;

    FileSizeType GetFileSize(const FileFDType fd);
    FileSizeType GetDataChunkSize(const FileFDType fd);
    MetaLpnType GetFileBaseLpn(const FileFDType fd);

    void CreateInitialInodeContent(uint32_t maxInodeNum);
    bool LoadInodeContent(void);
    MetaLpnType GetRegionBaseLpn(MetaRegionType regionType);
    MetaLpnType GetRegionSizeInLpn(MetaRegionType regionType);
    void PopulateFDMapWithVolumeType(std::unordered_map<FileFDType, MetaVolumeType>& dest);
    void PopulateFileNameWithVolumeType(std::unordered_map<StringHashType, MetaVolumeType>& dest);
    void PopulateFileKeyWithFD(std::unordered_map<StringHashType, FileFDType>& dest);
    FileFDType AllocNewFD(std::string& fileName);

    bool CreateFileInode(MetaFsMoMReqMsg& req, FileFDType newFd, MetaFilePageMap& pageMap, FileSizeType dataChunkSizeInMetaPage);
    bool DeleteFileInode(FileFDType& fd);
    bool IsFileInodeExist(std::string& fileName);
    MetaFileExtentContent* GetInodeHdrExtentMapBase(void);
    size_t GetInodeHdrExtentMapSize(void);
    void RemoveFDsInUse(std::map<FileFDType, FileFDType>& dstFreeFDMap);
    MetaFileInode& GetFileInode(const FileFDType fd);
    MetaFileInode& GetInodeEntry(const uint32_t entryIdx);
    bool IsFileInodeInUse(const FileFDType fd);
    size_t GetTotalAllocatedInodeCnt(void);

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(void);
#endif

    bool BackupContent(MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableLpnCnts);
    bool RestoreContent(MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts, MetaLpnType iNodeTableLpnCnts);

private:
    MetaFileInode& _AllocNewInodeEntry(FileFDType& newFd);
    void _UpdateFd2InodeMap(FileFDType fd, MetaFileInode& inode);
    void _BuildF2InodeMap(void);
    bool _LoadInodeFromMedia(MetaStorageType media, MetaLpnType baseLpn);
    bool _StoreInodeToMedia(MetaStorageType media, MetaLpnType baseLpn);

    MetaFileInodeTableHdr* inodeHdr;
    MetaFileInodeTable* inodeTable;
    std::unordered_map<FileFDType, MetaFileInode*> fd2InodeMap;
};
