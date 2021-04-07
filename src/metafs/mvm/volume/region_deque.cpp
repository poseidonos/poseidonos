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

#include "region_deque.h"

bool
RegionDeque::InsertUsedRange(MetaFileInode* inode, uint64_t baseLpn, uint64_t count, bool inUse)
{
    if (true == inUse)
    {
        remainedMetaLpnCount -= count;
    }
    regionList.push_back(Region(regionType, inode, baseLpn, count, inUse));

    return true;
}

bool
RegionDeque::InsertDone(void)
{
    uint32_t qSize = regionList.size();

    if (0 == qSize)
    {
        return false;
    }

    SetReducedLpnCount(0);

    sort(regionList.begin(), regionList.end());

    for (uint32_t idx = 0; idx < qSize - 1; idx++)
    {
        if (regionList[idx].GetLastMetaLpn() < (regionList[idx + 1].GetBaseMetaLpn() - 1))
        {
            uint64_t baseLpn = regionList[idx].GetLastMetaLpn() + 1;
            uint64_t count = regionList[idx + 1].GetBaseMetaLpn() - regionList[idx].GetLastMetaLpn() - 1;
            InsertUsedRange(nullptr, baseLpn, count, false);
            SetCompactionRequired(true);
        }
    }

    sort(regionList.begin(), regionList.end());

    qSize = regionList.size();
    if (maxMetaLpn > regionList[qSize - 1].GetLastMetaLpn())
    {
        uint64_t baseLpn = regionList[qSize - 1].GetLastMetaLpn() + 1;
        uint64_t count = maxMetaLpn - regionList[qSize - 1].GetLastMetaLpn();
        InsertUsedRange(nullptr, baseLpn, count, false);
    }

    return true;
}

pair<bool, uint64_t>
RegionDeque::GetTargetRegion(uint64_t baseLpn, uint64_t count)
{
    for (auto iter : regionList)
    {
        if (iter.GetInUse() == false && iter.GetSize() > count && iter.GetBaseMetaLpn() < baseLpn)
        {
            return make_pair(true, iter.GetBaseMetaLpn());
        }
    }

    return make_pair(false, 0);
}

bool
RegionDeque::Compaction(void)
{
    size_t qSize = regionList.size();
    bool isDone = false;

    for (uint32_t unusedIndex = 0; unusedIndex < qSize; unusedIndex++)
    {
        if (true == regionList[unusedIndex].GetInUse())
        {
            continue;
        }

        for (uint32_t usedIndex = qSize - 1; usedIndex > unusedIndex; usedIndex--)
        {
            if (regionList[unusedIndex].GetSize() >= regionList[usedIndex].GetSize() && true == regionList[usedIndex].GetInUse())
            {
                regionList[usedIndex].Move(&regionList[unusedIndex]);
                AppendReducedLpnCount(regionList[usedIndex].GetSize());
                isDone = true;
                break;
            }
        }
    }

    if (false == isDone)
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_COMPACTION_FAILED,
            "Compaction couldn't be done due to not enough free space");
    }

    SetCompactionRequired(false);
    sort(regionList.begin(), regionList.end());

    return isDone;
}

void
RegionDeque::UpdateExtentsList(MetaFileExtentContent* allocExtentsList)
{
    uint32_t newItemIdx = 0;

    memset(allocExtentsList, 0x0,
        sizeof(MetaFileExtentContent) * MetaFsConfig::MAX_VOLUME_CNT);

    if (0 == regionList.size())
        return;

    MetaLpnType start = regionList[0].GetBaseMetaLpn();
    MetaLpnType count = regionList[0].GetSize();

    for (uint32_t idx = 1; idx < regionList.size(); idx++)
    {
        if (!regionList[idx].GetInUse())
            continue;

        if (start + count == regionList[idx].GetBaseMetaLpn())
        {
            count += regionList[idx].GetSize();
        }
        else
        {
            allocExtentsList[newItemIdx].SetStartLpn(start);
            allocExtentsList[newItemIdx].SetCount(count);
            newItemIdx++;

            start = regionList[idx].GetBaseMetaLpn();
            count = regionList[idx].GetSize();
        }
    }
    allocExtentsList[newItemIdx].SetStartLpn(start);
    allocExtentsList[newItemIdx].SetCount(count);
}

void
RegionDeque::ReadRegions(void)
{
    MetaInodeTableArray& inodes = tableContent->entries;

    regionList.clear();

    for (auto& inode : inodes)
    {
        if (true == inode.IsInUse())
        {
            InsertUsedRange(&inode,
                (uint64_t)inode.data.basic.field.pagemap.baseMetaLpn,
                (uint64_t)inode.data.basic.field.pagemap.pageCnt);
        }
    }
}
