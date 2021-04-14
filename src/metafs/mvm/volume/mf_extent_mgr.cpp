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

#include "mf_extent_mgr.h"

#include "src/logger/logger.h"

namespace pos
{
MetaFileExtentManager::MetaFileExtentManager(void)
: fileRegionBaseLpnInVolume(MetaFsCommonConst::INVALID_META_LPN),
  maxFileRegionLpn(MetaFsCommonConst::INVALID_META_LPN),
  availableLpnCount(0)
{
}

MetaFileExtentManager::~MetaFileExtentManager(void)
{
}

void
MetaFileExtentManager::Init(MetaLpnType baseLpn, MetaLpnType maxFileRegionLpn)
{
    fileRegionBaseLpnInVolume = baseLpn;
    this->maxFileRegionLpn = maxFileRegionLpn;
    availableLpnCount = maxFileRegionLpn - fileRegionBaseLpnInVolume + 1;

    freeExtentsList.push_front(MetaFileExtent(fileRegionBaseLpnInVolume, availableLpnCount));

    MFS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "MetaFileExtentManager::Init fileRegionBaseLpnInVolume={}, maxFileRegionLpn={}, availableLpnCount={}",
        fileRegionBaseLpnInVolume, maxFileRegionLpn, availableLpnCount);
}

MetaFilePageMap
MetaFileExtentManager::AllocExtent(MetaLpnType lpnCnt)
{
    pair<bool, MetaLpnType> ret = AddToFreeExtentsList(lpnCnt);

    MetaFilePageMap pagemap;

    if (true == ret.first)
    {
        pagemap.baseMetaLpn = ret.second;
        pagemap.pageCnt = lpnCnt;
    }
    else
    {
        pagemap.baseMetaLpn = 0;
        pagemap.pageCnt = 0;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "New File pagemap info <baseLpn, lpnCnt>=<{}, {}>",
        pagemap.baseMetaLpn, pagemap.pageCnt);
    return pagemap;
}

MetaLpnType
MetaFileExtentManager::GetAvailableLpnCount(void)
{
    return availableLpnCount;
}

void
MetaFileExtentManager::GetContent(MetaFileExtent* list)
{
    int index = 0;

    memset(list, 0x0, sizeof(MetaFileExtent) * MetaFsConfig::MAX_VOLUME_CNT);

    MakeAllocatedExtentsList();

    for (auto iter = allocatedExtentsList.begin(); iter != allocatedExtentsList.end(); iter++)
    {
        MFS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "copy content from allocated list to outside: index={} iter->GetStartLpn()={}, iter->GetCount()={}",
            index, iter->GetStartLpn(), iter->GetCount());

        list[index].SetStartLpn(iter->GetStartLpn());
        list[index++].SetCount(iter->GetCount());
    }
}

void
MetaFileExtentManager::SetContent(MetaFileExtent* list)
{
    if (list[0].GetCount() == 0)
    {
        MFS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE, "There is no content in the list");
        return;
    }

    allocatedExtentsList.clear();

    // get allocated extent
    for (uint32_t index = 0; index < MetaFsConfig::MAX_VOLUME_CNT; index++)
    {
        if (list[index].GetCount() == 0)
        {
            break;
        }

        MFS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "copy content from outside to allocated list: index={} list[index].GetStartLpn()={}, list[index].GetCount()={}",
            index, list[index].GetStartLpn(), list[index].GetCount());

        AddToAllocatedExtentsList(list[index].GetStartLpn(), list[index].GetCount());
    }

    MakeFreeExtentsList();
    PrintFreeExtentsList();
}

MetaLpnType
MetaFileExtentManager::GetTheBiggestExtentSize(void)
{
    MetaLpnType size = 0;

    for (auto iter = freeExtentsList.begin(); iter != freeExtentsList.end(); iter++)
    {
        if (size < iter->GetCount())
        {
            size = iter->GetCount();
        }
    }

    return size;
}

void
MetaFileExtentManager::SetFileBaseLpn(MetaLpnType BaseLpn)
{
    fileRegionBaseLpnInVolume = BaseLpn;
    availableLpnCount = maxFileRegionLpn - fileRegionBaseLpnInVolume + 1;

    MFS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "set file base lpn, availableLpnCount={}", availableLpnCount);

    freeExtentsList.clear();
    freeExtentsList.push_front(MetaFileExtent(fileRegionBaseLpnInVolume, availableLpnCount));
    PrintFreeExtentsList();
}

pair<bool, MetaLpnType>
MetaFileExtentManager::AddToFreeExtentsList(MetaLpnType requestLpnCount)
{
    int index = 0;
    for (auto iter = freeExtentsList.begin(); iter != freeExtentsList.end(); iter++, index++)
    {
        if (iter->GetCount() > requestLpnCount)
        {
            MetaLpnType newStartLpn = freeExtentsList[index].GetStartLpn();

            freeExtentsList[index].SetStartLpn(freeExtentsList[index].GetStartLpn() + requestLpnCount);
            freeExtentsList[index].SetCount(freeExtentsList[index].GetCount() - requestLpnCount);
            availableLpnCount -= requestLpnCount;

            SortFreeExtentsList();
            PrintFreeExtentsList();

            return make_pair(true, newStartLpn);
        }
        else if (iter->GetCount() == requestLpnCount)
        {
            MetaLpnType newStartLpn = freeExtentsList[index].GetStartLpn();

            freeExtentsList.erase(iter);
            availableLpnCount -= requestLpnCount;

            SortFreeExtentsList();
            PrintFreeExtentsList();

            return make_pair(true, newStartLpn);
        }
    }

    return make_pair(false, 0);
}

bool
MetaFileExtentManager::RemoveFromFreeExtentsList(MetaLpnType startLpn, MetaLpnType count)
{
    if (count == 0)
    {
        return false;
    }

    MetaLpnType endLpn = (startLpn + count - 1);
    int index = 0;
    for (auto iter = freeExtentsList.begin(); iter != freeExtentsList.end(); iter++, index++)
    {
        if (iter->GetStartLpn() == (endLpn + 1))
        {
            freeExtentsList[index].SetStartLpn(freeExtentsList[index].GetStartLpn() - count);
            freeExtentsList[index].SetCount(freeExtentsList[index].GetCount() + count);
            availableLpnCount += count;
            SortFreeExtentsList();
            MergeFreeExtents();

            return true;
        }
        else if (startLpn == (iter->GetStartLpn() + iter->GetCount()))
        {
            iter->SetCount(iter->GetCount() + count);
            availableLpnCount += count;
            SortFreeExtentsList();
            MergeFreeExtents();

            return true;
        }
        else if (startLpn >= iter->GetStartLpn() && startLpn < (iter->GetStartLpn() + iter->GetCount()))
        {
            return false;
        }
    }

    freeExtentsList.push_back(MetaFileExtent(startLpn, count));
    availableLpnCount += count;
    SortFreeExtentsList();
    MergeFreeExtents();

    return true;
}

void
MetaFileExtentManager::MergeFreeExtents(void)
{
    uint32_t index = 0;
    bool eraseNext = false;

    for (auto iter = freeExtentsList.begin(); iter != freeExtentsList.end(); iter++, index++)
    {
        if (eraseNext == true)
        {
            freeExtentsList.erase(iter);
            eraseNext = false;

            if (freeExtentsList.size() <= index)
            {
                break;
            }
            continue;
        }

        if (freeExtentsList.size() != index + 1)
        {
            if ((iter->GetStartLpn() + iter->GetCount()) == freeExtentsList[index + 1].GetStartLpn())
            {
                iter->SetCount(iter->GetCount() + freeExtentsList[index + 1].GetCount());
                eraseNext = true;
            }
        }
    }
}

void
MetaFileExtentManager::MakeFreeExtentsList(void)
{
    uint32_t index = 0;
    MetaLpnType startLpn = 0;
    MetaLpnType count = 0;

    freeExtentsList.clear();
    availableLpnCount = 0;

    // get free extent
    for (auto iter = allocatedExtentsList.begin(); iter != allocatedExtentsList.end(); iter++, index++)
    {
        // first
        if (index == 0)
        {
            startLpn = fileRegionBaseLpnInVolume;
            count = iter->GetStartLpn() - fileRegionBaseLpnInVolume;

            if (count != 0)
            {
                availableLpnCount += count;
                freeExtentsList.push_back(MetaFileExtent(startLpn, count));
            }
        }

        // last
        if (allocatedExtentsList.size() == index + 1)
        {
            startLpn = iter->GetStartLpn() + iter->GetCount();
            count = maxFileRegionLpn - startLpn + 1;
        }
        else
        {
            startLpn = iter->GetStartLpn() + iter->GetCount();
            count = allocatedExtentsList[index + 1].GetStartLpn() - startLpn;
        }

        if (count != 0)
        {
            availableLpnCount += count;
            freeExtentsList.push_back(MetaFileExtent(startLpn, count));
        }
    }

    MergeFreeExtents();
}

void
MetaFileExtentManager::SortFreeExtentsList(void)
{
    sort(freeExtentsList.begin(), freeExtentsList.end());
}

void
MetaFileExtentManager::PrintFreeExtentsList(void)
{
    int totalCount = 0;

    MFS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "free space -> {}",
        availableLpnCount);

    for (MetaFileExtent extent : freeExtentsList)
    {
        totalCount += extent.GetCount();
        MFS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "({}, {})", extent.GetStartLpn(), extent.GetCount());
    }

    MFS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "total free lpn count: {} -> calculated)", totalCount);
}

void
MetaFileExtentManager::AddToAllocatedExtentsList(MetaLpnType startLpn, MetaLpnType requestLpnCount)
{
    allocatedExtentsList.push_back(MetaFileExtent(startLpn, requestLpnCount));
    SortAllocatedExtentsList();
}

void
MetaFileExtentManager::MakeAllocatedExtentsList(void)
{
    uint32_t index = 0;
    MetaLpnType startLpn = 0;
    MetaLpnType count = 0;

    allocatedExtentsList.clear();

    for (auto iter = freeExtentsList.begin(); iter != freeExtentsList.end(); iter++, index++)
    {
        // first
        if (index == 0)
        {
            startLpn = fileRegionBaseLpnInVolume;
            count = iter->GetStartLpn() - fileRegionBaseLpnInVolume;

            if (count != 0)
            {
                allocatedExtentsList.push_back(MetaFileExtent(startLpn, count));
            }
        }

        // last
        if (freeExtentsList.size() == index + 1)
        {
            startLpn = iter->GetStartLpn() + iter->GetCount();
            if (startLpn > maxFileRegionLpn)
                count = 0;
            else
                count = maxFileRegionLpn - startLpn + 1;
        }
        else
        {
            startLpn = iter->GetStartLpn() + iter->GetCount();
            count = freeExtentsList[index + 1].GetStartLpn() - startLpn;
        }

        if (count != 0)
        {
            allocatedExtentsList.push_back(MetaFileExtent(startLpn, count));
        }
    }

    PrintAllocatedExtentsList();
}

void
MetaFileExtentManager::SortAllocatedExtentsList(void)
{
    sort(allocatedExtentsList.begin(), allocatedExtentsList.end());
}

void
MetaFileExtentManager::PrintAllocatedExtentsList(void)
{
    int totalCount = 0;

    POS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "allocated space");

    for (MetaFileExtent extent : allocatedExtentsList)
    {
        totalCount += extent.GetCount();
        POS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "({}, {})", extent.GetStartLpn(), extent.GetCount());
    }

    POS_TRACE_DEBUG(POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "total allocated lpn count: {} -> calculated)", totalCount);
}
} // namespace pos
