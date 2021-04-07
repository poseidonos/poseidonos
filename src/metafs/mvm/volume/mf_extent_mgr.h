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

#include "mf_extent.h"
#include "mf_pagemap.h"

class MetaFileExtentMgrClass
{
public:
    MetaFileExtentMgrClass(void);
    ~MetaFileExtentMgrClass(void);

    void Init(MetaLpnType baseLpn, MetaLpnType maxFileRegionLpn);
    MetaFilePageMap AllocExtent(MetaLpnType lpnCnt);
    MetaLpnType GetAvailableLpnCount(void);
    MetaLpnType GetTheBiggestExtentSize(void);

    void GetContent(MetaFileExtentContent* list);
    void SetContent(MetaFileExtentContent* list);
    size_t GetContentSize(void);
    void SetFileBaseLpn(MetaLpnType BaseLpn);
    MetaLpnType
    GetFileBaseLpn(void)
    {
        return fileRegionBaseLpnInVolume;
    }

    /* to control free extents */
    pair<bool, MetaLpnType> AddToFreeExtentsList(MetaLpnType requestLpnCount);
    bool RemoveFromFreeExtentsList(MetaLpnType startLpn, MetaLpnType count);
    void MergeFreeExtents(void);
    void MakeFreeExtentsList(void);
    void SortFreeExtentsList(void);
    void PrintFreeExtentsList(void);

    void AddToAllocatedExtentsList(MetaLpnType startLpn, MetaLpnType requestLpnCount);
    void MakeAllocatedExtentsList(void);
    void SortAllocatedExtentsList(void);
    void PrintAllocatedExtentsList(void);

private:
    MetaLpnType fileRegionBaseLpnInVolume;
    MetaLpnType maxFileRegionLpn;
    MetaLpnType availableLpnCount;

    deque<MetaFileExtentContent> freeExtentsList;
    deque<MetaFileExtentContent> allocatedExtentsList;
};
