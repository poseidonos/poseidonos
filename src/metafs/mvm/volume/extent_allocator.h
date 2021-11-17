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

#include <algorithm>
#include <deque>
#include <utility>
#include <vector>

#include "src/metafs/include/meta_file_extent.h"

namespace pos
{
class ExtentAllocator
{
public:
    ExtentAllocator(void);
    virtual ~ExtentAllocator(void);

    virtual void Init(MetaLpnType _base, MetaLpnType _last);
    virtual std::vector<MetaFileExtent> AllocExtents(MetaLpnType lpnCnt);
    virtual MetaLpnType GetAvailableLpnCount(void);

    virtual std::vector<MetaFileExtent> GetAllocatedExtentList(void);
    virtual void SetAllocatedExtentList(std::vector<MetaFileExtent>& list);
    virtual void SetFileBaseLpn(MetaLpnType BaseLpn);
    virtual MetaLpnType
    GetFileBaseLpn(void)
    {
        return fileRegionBaseLpnInVolume;
    }

    virtual bool AddToFreeList(MetaLpnType startLpn, MetaLpnType count);
    virtual void PrintFreeExtentsList(void);

protected:
    void _SortAndCalcAvailable(bool needToAdd, MetaLpnType size);
    bool _RemoveFreeRange(MetaLpnType _start, MetaLpnType _count);

    MetaLpnType fileRegionBaseLpnInVolume;
    MetaLpnType maxFileRegionLpn;
    MetaLpnType availableLpnCount;

    std::deque<MetaFileExtent> freeList;
};
} // namespace pos
