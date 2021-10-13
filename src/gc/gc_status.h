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

#include <sys/time.h>

#include <cstdint>
#include <queue>

#include "src/lib/singleton.h"

namespace pos
{
class CopyInfo
{
public:
    explicit CopyInfo(uint32_t victimSegment);
    ~CopyInfo(void);
    int SetInfo(uint32_t invalidCnt, uint32_t copyDoneCnt);

    uint32_t
    GetSegmentId(void)
    {
        return segmentId;
    }
    uint32_t
    GetInvalidBlkCnt(void)
    {
        return invalidBlkCnt;
    }
    uint32_t
    GetCopiedBlkCnt(void)
    {
        return copiedBlkCnt;
    }
    struct timeval
    GetStartTime(void)
    {
        return startTime;
    }
    struct timeval
    GetEndTime(void)
    {
        return endTime;
    }

private:
    uint32_t segmentId = 0;
    uint32_t invalidBlkCnt = 0;
    uint32_t copiedBlkCnt = 0;
    struct timeval startTime = {
        0,
    };
    struct timeval endTime = {
        0,
    };
};

class GcStatus
{
public:
    GcStatus(void);
    ~GcStatus(void);

    int SetCopyInfo(bool started, uint32_t victimSegment,
        uint32_t invalidCnt, uint32_t copyDoneCnt);
    std::queue<CopyInfo>
    GetCopyInfoList(void)
    {
        return copyInfoList;
    }
    bool
    GetGcRunning(void)
    {
        return gcRunning;
    }

    struct timeval
    GetStartTime(void)
    {
        return startTime;
    }
    struct timeval
    GetEndTime(void)
    {
        return endTime;
    }

private:
    bool gcRunning;
    uint32_t logCount = 30;
    std::queue<CopyInfo> copyInfoList;
    CopyInfo currentCopyInfo;

    struct timeval startTime;
    struct timeval endTime;
};

} // namespace pos
