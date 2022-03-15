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

#include <array>
#include <queue>
#include <vector>

#include "file_descriptor_in_use_map.h"
#include "metafs_common.h"
#include "on_volume_meta_region.h"
#include "src/metafs/include/meta_file_extent.h"

namespace pos
{
class InodeInUseBitmap
{
public:
    InodeInUseBitmap&
    operator=(const InodeInUseBitmap& src)
    {
        bits = src.bits;
        return *this;
    }

    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bits;
    uint32_t allocatedInodeCnt;
};

class InodeTableHeaderContent : public MetaRegionContent
{
public:
    uint32_t totalInodeNum;
    size_t inodeEntryByteSize;
    uint32_t totalFileCreated;
    InodeInUseBitmap inodeInUseBitmap;
    std::array<MetaFileExtent, MetaFsConfig::MAX_VOLUME_CNT> allocExtentsList;
};

class InodeTableHeader : public OnVolumeMetaRegion<MetaRegionType, InodeTableHeaderContent>
{
public:
    explicit InodeTableHeader(MetaVolumeType volumeType, MetaLpnType baseLpn);
    virtual ~InodeTableHeader(void);

    virtual void Create(uint32_t totalFileNum);
    virtual void SetInodeInUse(uint32_t idx);
    virtual void ClearInodeInUse(uint32_t idx);
    virtual bool IsFileInodeInUse(uint32_t idx);
    virtual uint32_t GetTotalAllocatedInodeCnt(void);
    virtual std::vector<MetaFileExtent> GetFileExtentContent(void);
    virtual void SetFileExtentContent(std::vector<MetaFileExtent>& extents);
    virtual size_t GetFileExtentContentSize(void);
    virtual void BuildFreeInodeEntryMap(void);
    virtual uint32_t GetFreeInodeEntryIdx(void);
    virtual std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& GetInodeInUseBitmap(void);
    virtual bool Load(void);
    virtual bool Load(MetaStorageType media, MetaLpnType baseLPN,
        uint32_t idx, MetaLpnType pageCNT);
    virtual bool Store(void);
    virtual bool Store(MetaStorageType media, MetaLpnType baseLPN,
        uint32_t idx, MetaLpnType pageCNT);

private:
    void _PrintLog(void) const;
    std::queue<uint32_t>* freeInodeEntryIdxQ;
};
} // namespace pos
