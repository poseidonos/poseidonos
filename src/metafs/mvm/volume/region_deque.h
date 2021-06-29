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

#include <algorithm>
#include <deque>
#include <utility>
#include <string>

#include "mdpage.h"
#include "inode_table.h"
#include "region.h"
#include "mf_extent.h"

namespace pos
{
class RegionDeque
{
public:
    RegionDeque(int arrayId, MetaStorageType type, InodeTableContent* content, uint64_t maxLpnCount, MetaStorageSubsystem* metaStorage)
    {
        this->arrayId = arrayId;
        regionType = type;
        maxMetaLpn = maxLpnCount - 1;
        remainedMetaLpnCount = maxMetaLpn + 1;
        tableContent = content;
        needCompaction = false;
        reducedLpnCount = 0;
        this->metaStorage = metaStorage;
    }

    bool InsertUsedRange(MetaFileInode* inode, uint64_t baseLpn, uint64_t count, bool inUse = true);
    bool InsertDone(void);
    pair<bool, uint64_t> GetTargetRegion(uint64_t baseLpn, uint64_t count);
    bool Compaction(void);
    void UpdateExtentsList(MetaFileExtent* allocExtentsList);
    void
    SetCompactionRequired(bool flag)
    {
        needCompaction = flag;
    }
    bool
    IsCompactionRequired(void)
    {
        return needCompaction;
    }
    void ReadRegions(void);
    void
    SetReducedLpnCount(uint64_t count)
    {
        reducedLpnCount = count;
    }
    void
    AppendReducedLpnCount(uint64_t count)
    {
        reducedLpnCount += count;
    }
    uint64_t
    GetReducedLpnCount(void)
    {
        return reducedLpnCount;
    }
    uint64_t
    GetRemainedMetLpnCount(void)
    {
        return remainedMetaLpnCount;
    }

private:
    deque<Region> regionList;
    MetaStorageType regionType;
    uint64_t maxMetaLpn;
    uint64_t remainedMetaLpnCount;
    bool needCompaction;
    InodeTableContent* tableContent;
    uint64_t reducedLpnCount;
    int arrayId;
    MetaStorageSubsystem* metaStorage;
};
} // namespace pos
