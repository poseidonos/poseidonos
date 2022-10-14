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

#include "event_mpage_async_io.h"

#include "map_io_handler.h"

namespace pos
{
EventMpageAsyncIo::EventMpageAsyncIo(MapHeader* mapHeader_, Map* map_,
    MetaFileIntf* file_, MetaIoCbPtr asyncIoReqCB_)
{
    mapHeader = mapHeader_;
    map = map_;
    file = file_;
    asyncIoReqCB = asyncIoReqCB_;
    startMpage = 0;
}

// LCOV_EXCL_START
EventMpageAsyncIo::~EventMpageAsyncIo(void)
{
}
// LCOV_EXCL_STOP

bool
EventMpageAsyncIo::Execute(void)
{
    int mpageNum = startMpage;
    uint64_t numBitsSet = mapHeader->GetMpageMap()->GetNumBitsSet();

    for (uint32_t cnt = startMpage; cnt < numBitsSet; ++cnt)
    {
        mpageNum = mapHeader->GetMpageMap()->FindFirstSet(mpageNum);
        char* mpage = map->AllocateMpage(mpageNum);

        MapFlushIoContext* mPageLoadRequest = new MapFlushIoContext();
        mPageLoadRequest->opcode = MetaFsIoOpcode::Read;
        mPageLoadRequest->fd = file->GetFd();
        mPageLoadRequest->fileOffset = mapHeader->GetSize() + (mpageNum * map->GetSize());
        mPageLoadRequest->length = map->GetSize();
        mPageLoadRequest->buffer = mpage;
        mPageLoadRequest->callback = asyncIoReqCB;
        mPageLoadRequest->startMpage = mpageNum;
        mPageLoadRequest->numMpages = 1;

        if (file->AsyncIO(mPageLoadRequest) < 0) // MFS_FAILED_DUE_TO_ERR
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper AsyncLoad] Failed to Issue AsyncLoad, retry.. mpageNum:{}", mpageNum);
            POS_TRACE_ERROR(EID(MAPPER_FAILED), mPageLoadRequest->ToString());
            startMpage = mpageNum;
            return false;
        }
        mpageNum++;
    }

    return true;
}

} // namespace pos

