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

#include "event_mpage_async_io.h"

#include "map_flush_handler.h"

namespace pos
{
EventMpageAsyncIo::EventMpageAsyncIo(MapHeader* mapHeader, Map* map,
    MetaFileIntf* file, MetaIoCbPtr asyncIoReqCB)
{
    mapHeader_ = mapHeader;
    map_ = map;
    file_ = file;
    asyncIoReqCB_ = asyncIoReqCB;
}

bool
EventMpageAsyncIo::Execute(void)
{
    int mpageNum = 0;
    uint64_t numBitsSet = mapHeader_->GetMpageMap()->GetNumBitsSet();

    for (uint32_t cnt = 0; cnt < numBitsSet; ++cnt)
    {
        mpageNum = mapHeader_->GetMpageMap()->FindFirstSet(mpageNum);
        char* mpage = map_->AllocateMpage(mpageNum);

        MapFlushIoContext* mPageLoadRequest = new MapFlushIoContext();
        mPageLoadRequest->opcode = MetaFsIoOpcode::Read;
        mPageLoadRequest->fd = file_->GetFd();
        mPageLoadRequest->fileOffset = mapHeader_->GetSize() + (mpageNum * mapHeader_->GetMpageSize());
        mPageLoadRequest->length = mapHeader_->GetMpageSize();
        mPageLoadRequest->buffer = mpage;
        mPageLoadRequest->callback = asyncIoReqCB_;
        mPageLoadRequest->startMpage = mpageNum;
        mPageLoadRequest->numMpages = 1;

        file_->AsyncIO(mPageLoadRequest);
        mpageNum++;
    }

    return true;
}

} // namespace pos

