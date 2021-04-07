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

#include "mapper_mock.h"

#include <thread>

MockMapper::MockMapper(TestInfo* _testInfo)
{
    testInfo = _testInfo;
    flushHandler.resize(testInfo->numMap);
    for (int mapId = 0; mapId < testInfo->numMap; mapId++)
    {
        flushHandler[mapId] = new MockMapFlushHandler(mapId);
    }

    stripeMapFlushHandler = new MockMapFlushHandler(STRIPE_MAP_ID);

    stripeMap = new StripeAddr[testInfo->numUserStripes];
    for (uint32_t idx = 0; idx < testInfo->numUserStripes; idx++)
    {
        stripeMap[idx].stripeId = UNMAP_STRIPE;
    }

    vsaMap = new VirtualBlkAddr*[testInfo->maxNumVolume];
    for (int idx = 0; idx < testInfo->maxNumVolume; idx++)
    {
        vsaMap[idx] = new VirtualBlkAddr[testInfo->maxVolumeSizeInBlock];
        for (uint64_t blk = 0; blk < testInfo->maxVolumeSizeInBlock; blk++)
        {
            vsaMap[idx][blk] = UNMAP_VSA;
        }
    }

    ON_CALL(*this, StartDirtyPageFlush).WillByDefault(::testing::Invoke(this, &MockMapper::_StartDirtyPageFlush));
    ON_CALL(*this, UpdateStripeMap).WillByDefault(::testing::Invoke(this, &MockMapper::_UpdateStripeMap));
    ON_CALL(*this, GetLSA).WillByDefault(::testing::Invoke(this, &MockMapper::_GetLsid));
    ON_CALL(*this, GetVSAInternal).WillByDefault(::testing::Invoke(this, &MockMapper::_GetVSAInternal));
    ON_CALL(*this, SetVsaMapInternal).WillByDefault(::testing::Invoke(this, &MockMapper::_SetVsaMapInternal));
}

MockMapper::~MockMapper(void)
{
    for (int mapId = 0; mapId < testInfo->numMap; mapId++)
    {
        delete flushHandler[mapId];
    }
    delete stripeMapFlushHandler;
}

MpageList
MockMapper::GetVsaMapDirtyPages(int volId, BlkAddr rba, uint32_t numBlks)
{
    MpageList dirty;
    int numEntriesPerPage = testInfo->metaPageSize / 8;

    MpageNum startMpage = rba / numEntriesPerPage;
    MpageNum endMpage = (rba + numBlks - 1) / numEntriesPerPage;

    for (uint64_t page = startMpage; page <= endMpage; page++)
    {
        MpageNum pageNum = (int)rba / numEntriesPerPage;
        dirty.insert(pageNum);
    }
    return dirty;
}

MpageList
MockMapper::GetStripeMapDirtyPages(StripeId vsid)
{
    int numEntriesPerPage = testInfo->metaPageSize / 4;

    MpageList dirty;
    MpageNum pageNum = vsid / numEntriesPerPage;
    dirty.insert(pageNum);

    return dirty;
}

int
MockMapper::_UpdateStripeMap(StripeId vsid, StripeId lsid, StripeLoc loc)
{
    assert(vsid < testInfo->numUserStripes);

    StripeAddr entry = {.stripeLoc = loc, .stripeId = lsid};
    stripeMap[vsid] = entry;
    return 0;
}

StripeAddr
MockMapper::_GetLsid(StripeId vsid)
{
    assert(vsid < testInfo->numUserStripes);
    return stripeMap[vsid];
}

VirtualBlkAddr
MockMapper::_GetVSAInternal(int volumeId, BlkAddr rba, int& caller)
{
    assert(volumeId < testInfo->maxNumVolume);
    assert(rba < testInfo->maxVolumeSizeInBlock);

    caller = OK_READY;
    return vsaMap[volumeId][rba];
}

int
MockMapper::_SetVsaMapInternal(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    assert(volumeId < testInfo->maxNumVolume);

    for (uint32_t blkCount = 0; blkCount < virtualBlks.numBlks; blkCount++)
    {
        assert(startRba + blkCount < testInfo->maxVolumeSizeInBlock);
        vsaMap[volumeId][startRba + blkCount].stripeId = virtualBlks.startVsa.stripeId;
        vsaMap[volumeId][startRba + blkCount].offset = virtualBlks.startVsa.offset + blkCount;
    }
    return 0;
}

int
MockMapper::_StartDirtyPageFlush(int mapId, MpageList dirtyPages,
    EventSmartPtr callback)
{
    if (mapId == -1)
    {
        return stripeMapFlushHandler->StartDirtyPageFlush(dirtyPages, callback);
    }
    else
    {
        return flushHandler[mapId]->StartDirtyPageFlush(dirtyPages, callback);
    }
}

MockMapFlushHandler::MockMapFlushHandler(int mapId)
{
    id = mapId;
    numPagesToFlush = 0;
    numPagesFlushed = 0;
}

int
MockMapFlushHandler::StartDirtyPageFlush(MpageList dirtyPages,
    EventSmartPtr callback)
{
    std::this_thread::sleep_for(1s);

    callbackEvent = callback;
    numPagesToFlush = dirtyPages.size();
    for (auto it = dirtyPages.begin(); it != dirtyPages.end(); ++it)
    {
        std::thread flushDone(&MockMapFlushHandler::_MpageFlushed, this, *it);
        flushDone.detach();
    }
    return 0;
}

void
MockMapFlushHandler::_MpageFlushed(int pageId)
{
    std::unique_lock<std::mutex> l(lock);
    numPagesFlushed++;
    if (numPagesToFlush == numPagesFlushed)
    {
        callbackEvent->Execute();
        numPagesToFlush = 0;
        numPagesFlushed = 0;
    }
}
