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

#include "src/gc/gc_status.h"
#include "src/logger/logger.h"

namespace pos
{
CopyInfo::CopyInfo(uint32_t victimSegment)
: segmentId(victimSegment)
{
    gettimeofday(&startTime, NULL);
}

CopyInfo::~CopyInfo(void)
{
}
int
CopyInfo::SetInfo(uint32_t invalidCnt, uint32_t copyDoneCnt)
{
    gettimeofday(&endTime, NULL);
    invalidBlkCnt = invalidCnt;
    copiedBlkCnt = copyDoneCnt;

    return 0;
}

GcStatus::GcStatus(void)
: gcRunning(false),
  currentCopyInfo(0)
{
    startTime = {
        0,
    };
    endTime = {
        0,
    };
}

GcStatus::~GcStatus(void)
{
}

int
GcStatus::SetCopyInfo(bool started, uint32_t victimSegment,
    uint32_t invalidCnt, uint32_t copyDoneCnt)
{
    if (false == started)
    {
        CopyInfo copyInfo(victimSegment);
        currentCopyInfo = copyInfo;
        gettimeofday(&startTime, NULL);
        gcRunning = true;
    }
    else
    {
        currentCopyInfo.SetInfo(invalidCnt, copyDoneCnt);

        if (logCount < copyInfoList.size())
        {
            copyInfoList.pop();
        }
        copyInfoList.push(currentCopyInfo);

        gettimeofday(&endTime, NULL);
        gcRunning = false;
    }

    return 0;
}

} // namespace pos
