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

#include "src/mapper/include/mpage_info.h"
#include "src/lib/bitmap.h"

namespace pos
{
static const int MAX_MPAGES_PER_SET = 1024;

struct MpageSet
{
    MpageNum startMpage;
    int numMpages;

    bool
    CanBeCoalesced(MpageNum page)
    {
        return (startMpage - 1 <= page &&
            page <= startMpage + numMpages &&
            numMpages < MAX_MPAGES_PER_SET);
    }

    void
    Coalesce(MpageNum page)
    {
        if (startMpage - 1 == page)
        {
            startMpage--;
        }
        else if (page == startMpage + numMpages)
        {
            numMpages++;
        }
    }
};

class SequentialPageFinder
{
public:
    explicit SequentialPageFinder(MpageList& pages);
    explicit SequentialPageFinder(BitMap* pages);
    virtual ~SequentialPageFinder(void);

    MpageSet PopNextMpageSet(void);
    bool IsRemaining(void);

private:
    void _UpdateSequentialPageList(MpageList& pages);
    std::queue<MpageSet> sequentialPages;
};

} // namespace pos
