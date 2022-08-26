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

#include "extent_allocator.h"
#include "src/logger/logger.h"

#include <algorithm>

namespace pos
{
ExtentAllocator::ExtentAllocator(void)
: fileRegionBaseLpnInVolume(MetaFsCommonConst::INVALID_META_LPN),
  maxFileRegionLpn(MetaFsCommonConst::INVALID_META_LPN),
  availableLpnCount(0)
{
}

ExtentAllocator::~ExtentAllocator(void)
{
}

void
ExtentAllocator::Init(MetaLpnType _base, MetaLpnType _last)
{
    assert(_base < _last);

    fileRegionBaseLpnInVolume = _base;
    maxFileRegionLpn = _last;
    availableLpnCount = _last - _base + 1;

    freeList.push_back({ fileRegionBaseLpnInVolume, availableLpnCount });

    POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "ExtentAllocator::Init fileRegionBaseLpnInVolume={}, maxFileRegionLpn={}, availableLpnCount={}",
        fileRegionBaseLpnInVolume, maxFileRegionLpn, availableLpnCount);
}

std::vector<MetaFileExtent>
ExtentAllocator::AllocExtents(MetaLpnType lpnCnt)
{
    std::vector<MetaFileExtent> result;
    MetaLpnType newLpnCnt = lpnCnt;
    uint32_t remain = newLpnCnt % MetaFsConfig::LPN_COUNT_PER_EXTENT;

    if (0 != remain)
    {
        newLpnCnt += MetaFsConfig::LPN_COUNT_PER_EXTENT - remain;
    }

    if (availableLpnCount >= newLpnCnt)
    {
        MetaLpnType remainedCnt = newLpnCnt;

        for (auto item = freeList.begin(); item != freeList.end() && remainedCnt != 0; )
        {
            if ((*item).GetCount() <= remainedCnt)
            {
                remainedCnt -= (*item).GetCount();
                result.push_back(*item);
                item = freeList.erase(item);
            }
            else
            {
                (*item).SetCount((*item).GetCount() - remainedCnt);
                result.push_back({ (*item).GetStartLpn(), remainedCnt });
                (*item).SetStartLpn((*item).GetStartLpn() + remainedCnt);
                remainedCnt = 0;
            }
        }

        _SortAndCalcAvailable(false, newLpnCnt);
    }

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "[ExtentAllocator] base: {}, last: {}, available: {}",
        fileRegionBaseLpnInVolume, maxFileRegionLpn, availableLpnCount);
    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "[ExtentAllocator] requested lpn count: {}, allocated extent count: {}",
        lpnCnt, result.size());

    for (auto& extent : result)
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "allocated extent, startLpn: {}, lpnCount: {}",
            extent.GetStartLpn(), extent.GetCount());
    }

    if (result.size() > MetaFsConfig::MAX_PAGE_MAP_CNT)
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "It has exceeded the maximum extent count that can be allocated.");
        assert(0);
    }

    return result;
}

MetaLpnType
ExtentAllocator::GetAvailableLpnCount(void)
{
    return availableLpnCount;
}

std::vector<MetaFileExtent>
ExtentAllocator::GetAllocatedExtentList(void)
{
    std::vector<MetaFileExtent> allocList;

    if (freeList.size() == 0)
    {
        allocList.push_back({
                        fileRegionBaseLpnInVolume,
                        maxFileRegionLpn - fileRegionBaseLpnInVolume + 1 });
        return allocList;
    }
    else if (freeList.size() == 1)
    {
        MetaFileExtent& extent = freeList[0];
        MetaLpnType start = 0;
        MetaLpnType count = 0;

        if (extent.GetStartLpn() != fileRegionBaseLpnInVolume)
        {
            start = fileRegionBaseLpnInVolume;
            count = extent.GetStartLpn() - fileRegionBaseLpnInVolume;
        }
        else
        {
            start = extent.GetLast() + 1;
            count = maxFileRegionLpn - extent.GetLast();
        }

        allocList.push_back({ start, count });

        return allocList;
    }

    for (int i = 0; i < (int)freeList.size(); ++i)
    {
        MetaFileExtent& extent = freeList[i];
        MetaLpnType start = 0;
        MetaLpnType count = 0;

        // the first item
        if (i == 0)
        {
            if (extent.GetStartLpn() != fileRegionBaseLpnInVolume)
            {
                start = fileRegionBaseLpnInVolume;
                count = extent.GetStartLpn() - fileRegionBaseLpnInVolume;
            }
        }
        // the last item
        else if (i == (int)freeList.size() - 1)
        {
            if (extent.GetLast() != maxFileRegionLpn)
            {
                start = extent.GetLast() + 1;
                count = maxFileRegionLpn - extent.GetLast();
            }
            else
            {
                start = freeList[i - 1].GetLast() + 1;
                count = extent.GetStartLpn() - freeList[i - 1].GetLast() - 1;
            }
        }
        else
        {
            MetaFileExtent& prevExtent = freeList[i - 1];
            start = prevExtent.GetLast() + 1;
            count = extent.GetStartLpn() - prevExtent.GetLast() - 1;
        }

        if (count != 0)
            allocList.push_back({ start, count });
    }

    return allocList;
}

void
ExtentAllocator::SetAllocatedExtentList(std::vector<MetaFileExtent>& list)
{
    if (list.size() == 0)
    {
        POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "There is no content in the list");
        return;
    }

    // get allocated extent
    for (auto& extent : list)
    {
        POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "copy content from outside to allocated list: extent.GetStartLpn()={}, extent.GetCount()={}",
            extent.GetStartLpn(), extent.GetCount());

        _RemoveFreeRange(extent.GetStartLpn(), extent.GetCount());
    }
}

void
ExtentAllocator::SetFileBaseLpn(MetaLpnType BaseLpn)
{
    fileRegionBaseLpnInVolume = BaseLpn;
    availableLpnCount = maxFileRegionLpn - fileRegionBaseLpnInVolume + 1;

    freeList.clear();
    freeList.push_back({ fileRegionBaseLpnInVolume, availableLpnCount });

    POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "set file base lpn, availableLpnCount={}", availableLpnCount);
}

bool
ExtentAllocator::AddToFreeList(MetaLpnType _start, MetaLpnType _count)
{
    if (_count == 0)
    {
        return false;
    }

    if (freeList.size() == 0)
    {
        freeList.push_back({ _start, _count });

        _SortAndCalcAvailable(true, _count);
        return true;
    }

    MetaFileExtent item = MetaFileExtent(_start, _count);
    bool result = false;
    int i = 0;

    for (auto iter = freeList.begin(); iter != freeList.end(); ++iter, ++i)
    {
        MetaFileExtent& extent = *iter;

        // the first item & new extent is not connected with the first
        if ((i == 0) && (item.GetLast() + 1 < extent.GetStartLpn()))
        {
            freeList.push_back(item);
            result = true;
            break;
        }
        // the first item & new extent is connected with the first
        else if ((i == 0) && (item.GetLast() + 1 == extent.GetStartLpn()))
        {
            extent.SetStartLpn(item.GetStartLpn());
            extent.SetCount(extent.GetCount() + item.GetCount());

            result = true;
            break;
        }
        // new extent is not connected with the extent
        else if (item.GetLast() + 1 < extent.GetStartLpn())
        {
            freeList.push_back(item);
            result = true;
            break;
        }
        // new extent is connected with the extent
        else if (extent.GetLast() + 1 == item.GetStartLpn())
        {
            extent.SetCount(extent.GetCount() + item.GetCount());
            // new extent is connected with the next
            if ((iter + 1) != freeList.end())
            {
                MetaFileExtent& back = *(iter + 1);
                if (item.GetLast() + 1 == back.GetStartLpn())
                {
                    extent.SetCount(extent.GetCount() + back.GetCount());
                    freeList.erase(iter + 1);
                }
            }
            result = true;
            break;
        }
        // new extent is connected with the next
        else if ((iter + 1) != freeList.end())
        {
            MetaFileExtent& back = *(iter + 1);
            if (item.GetLast() + 1 == back.GetStartLpn())
            {
                back.SetStartLpn(item.GetStartLpn());
                back.SetCount(back.GetCount() + item.GetCount());
                result = true;
                break;
            }
        }
    }

    if (false == result)
    {
        freeList.push_back(item);
        result = true;
    }

    _SortAndCalcAvailable(true, _count);

    return result;
}

void
ExtentAllocator::_SortAndCalcAvailable(bool needToAdd, MetaLpnType size)
{
    std::sort(freeList.begin(), freeList.end());
    if (needToAdd)
        availableLpnCount += size;
    else
        availableLpnCount -= size;
}

bool
ExtentAllocator::_RemoveFreeRange(MetaLpnType _start, MetaLpnType _count)
{
    if (availableLpnCount == 0)
        return false;

    if (freeList.size() == 0)
    {
        if (_start != fileRegionBaseLpnInVolume)
            freeList.push_back({ fileRegionBaseLpnInVolume, _start });
        if (_start + _count - 1 < availableLpnCount - fileRegionBaseLpnInVolume)
            freeList.push_back({
                    _start + _count,
                    maxFileRegionLpn - (_start + _count - 1) });

        _SortAndCalcAvailable(false, _count);
        return true;
    }

    bool result = false;

    for (auto iter = freeList.begin(); iter != freeList.end(); ++iter)
    {
        MetaFileExtent& extent = *iter;

        // fit
        if ((extent.GetStartLpn() == _start) &&
            (extent.GetLast() == (_start + _count - 1)))
        {
            freeList.erase(iter);

            result = true;
            break;
        }
        // in boundary
        else if ((extent.GetStartLpn() <= _start) &&
                (extent.GetLast() >= (_start + _count - 1)))
        {
            MetaLpnType original_last = extent.GetLast();
            if (_start == extent.GetStartLpn())
            {
                extent.SetStartLpn(extent.GetStartLpn() + _count);
                extent.SetCount(extent.GetCount() - _count);

                result =  true;
                break;
            }
            else
            {
                extent.SetCount(_start - extent.GetStartLpn());
            }

            if (_start + _count - 1 != original_last)
            {
                freeList.push_back({
                            _start + _count,
                            original_last - _start - _count + 1 });
            }

            result = true;
            break;
        }
    }

    _SortAndCalcAvailable(false, _count);

    return result;
}

void
ExtentAllocator::PrintFreeExtentsList(void)
{
    int totalCount = 0;

    POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "free space -> {}",
        availableLpnCount);

    for (MetaFileExtent extent : freeList)
    {
        totalCount += extent.GetCount();
        POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "({}, {})", extent.GetStartLpn(), extent.GetCount());
    }

    POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "total free lpn count: {} -> calculated)", totalCount);
}
} // namespace pos
