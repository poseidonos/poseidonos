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

#include "gc_block_write_done_log_handler.h"

#include <string.h>

namespace pos
{
GcBlockWriteDoneLogHandler::GcBlockWriteDoneLogHandler(int volumeId, StripeId vsid, GcBlockMapUpdateList blockMapUpdateList)
{
    int numBlockMaps = blockMapUpdateList.size();
    logSize = sizeof(GcBlockWriteDoneLog) + sizeof(GcBlockMapUpdate) * numBlockMaps;

    dat = malloc(logSize);

    logPtr = new (dat) GcBlockWriteDoneLog;
    logPtr->type = LogType::GC_BLOCK_WRITE_DONE;
    logPtr->volId = volumeId;
    logPtr->vsid = vsid;
    logPtr->numBlockMaps = numBlockMaps;

    blockMapListPtr = reinterpret_cast<GcBlockMapUpdate*>((char*)dat + sizeof(GcBlockWriteDoneLog));
    GcBlockMapUpdate* currentPtr = blockMapListPtr;
    for (auto blockMapUpdates : blockMapUpdateList)
    {
        currentPtr->rba = blockMapUpdates.rba;
        currentPtr->vsa = blockMapUpdates.vsa;

        currentPtr++;
    }
}

GcBlockWriteDoneLogHandler::GcBlockWriteDoneLogHandler(void* inputData)
{
    GcBlockWriteDoneLog* inputLog = reinterpret_cast<GcBlockWriteDoneLog*>(inputData);
    logSize = sizeof(GcBlockWriteDoneLog) + sizeof(GcBlockMapUpdate) * inputLog->numBlockMaps;

    dat = malloc(logSize);
    memcpy(dat, inputData, logSize);

    logPtr = reinterpret_cast<GcBlockWriteDoneLog*>(dat);
    blockMapListPtr = reinterpret_cast<GcBlockMapUpdate*>((char*)dat + sizeof(GcBlockWriteDoneLog));
}

GcBlockWriteDoneLogHandler::~GcBlockWriteDoneLogHandler(void)
{
    if (dat != nullptr)
    {
        free(dat);

        dat = nullptr;
        blockMapListPtr = nullptr;
    }
}

bool
GcBlockWriteDoneLogHandler::operator==(const GcBlockWriteDoneLogHandler& log)
{
    bool isSameStripe = (logPtr->volId == log.logPtr->volId)
                    && (logPtr->vsid == log.logPtr->vsid)
                    && (logPtr->numBlockMaps == log.logPtr->numBlockMaps);

    if (isSameStripe == false)
    {
        return false;
    }
    else
    {
        for (int index = 0; index < logPtr->numBlockMaps; index++)
        {
            if ((blockMapListPtr[index].rba == log.blockMapListPtr[index].rba)
                && (blockMapListPtr[index].vsa == log.blockMapListPtr[index].vsa))
            {
                continue;
            }
            else
            {
                return false;
            }
        }
        return true;
    }
}

LogType
GcBlockWriteDoneLogHandler::GetType(void)
{
    return logPtr->type;
}

uint32_t
GcBlockWriteDoneLogHandler::GetSize(void)
{
    return logSize;
}

char*
GcBlockWriteDoneLogHandler::GetData(void)
{
    return (char*)dat;
}

StripeId
GcBlockWriteDoneLogHandler::GetVsid(void)
{
    return logPtr->vsid;
}

uint32_t
GcBlockWriteDoneLogHandler::GetSeqNum(void)
{
    return logPtr->seqNum;
}

void
GcBlockWriteDoneLogHandler::SetSeqNum(uint32_t num)
{
    logPtr->seqNum = num;
}

GcBlockWriteDoneLog*
GcBlockWriteDoneLogHandler::GetGcBlockMapWriteDoneLog(void)
{
    return logPtr;
}

GcBlockMapUpdate*
GcBlockWriteDoneLogHandler::GetMapList(void)
{
    return blockMapListPtr;
}
} // namespace pos
