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

#include <queue>

#include "meta_fd.h"
#include "mf_extent.h"
#include "mfs_common.h"
#include "on_volume_meta_region.h"

class MetaInodeEntryInUseBitmap
{
public:
    MetaInodeEntryInUseBitmap&
    operator=(const MetaInodeEntryInUseBitmap& src)
    {
        bits = src.bits;
        return *this;
    }

    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bits;
    uint32_t allocatedInodeCnt;
};

class MetaFileInodeTableHdrContent : public MetaFsContentAllocator
{
public:
    uint32_t totalInodeNum;
    size_t inodeEntryByteSize;
    uint32_t totalFileCreated;
    MetaInodeEntryInUseBitmap inodeInUseBitmap;
    MetaFileExtentContent allocExtentsList[MetaFsConfig::MAX_VOLUME_CNT];
};

class MetaFileInodeTableHdr : public OnVolumeMetaRegion<MetaRegionType, MetaFileInodeTableHdrContent>
{
public:
    explicit MetaFileInodeTableHdr(MetaVolumeType volumeType, MetaLpnType baseLpn);
    ~MetaFileInodeTableHdr(void);

    void Create(uint32_t totalFileNum);
    void SetInodeInUse(uint32_t idx);
    void ClearInodeInUse(uint32_t idx);
    bool IsFileInodeInUse(uint32_t idx);
    uint32_t GetTotalAllocatedInodeCnt(void);
    MetaFileExtentContent* GetFileExtentContentBase(void);
    size_t GetFileExtentContentSize(void);
    void BuildFreeInodeEntryMap(void);
    uint32_t GetFreeInodeEntryIdx(void);
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& GetInodeInUseBitmap(void);

private:
    std::queue<uint32_t>* freeInodeEntryIdxQ;
};
