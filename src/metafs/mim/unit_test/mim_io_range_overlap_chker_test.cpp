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

#include "mim_io_range_overlap_chker_test.h"

#include "mfs_io_config.h"
#include "mfs_io_range_overlap_chker.h"

MetaFsIoRangeOverlapChker globalIoRangeOverlapChker;

InstanceTagIdAllocator aiocbTagIdAllocator;

UtMIMRangeLockChker::UtMIMRangeLockChker(void)
{
}

UtMIMRangeLockChker::~UtMIMRangeLockChker(void)
{
}

// There are several cases.

/* Case 1: 
          (0)             (chunk size)
[Req1 W ]  ---------------------
[Req2 R]                  ------------------ 
[Req3 R]                       -------       

Req2 R : LPN overlapped 
Req1 W release from outstanding Q
Req3 R : no outstaing I/O count, so that can be executed
*/

TEST_F(UtMIMRangeLockChker, RangeOverlapChkTest1)
{
    FileFDType fd;
    const FileFDType invalidFD = MetaFsCommonConst::INVALID_FD;

    fd = OpenDummyFile();
    if (fd == invalidFD)
    {
        FileSizeType fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100; // 400KB
        fd = CreateFileAndOpen(fileSize);
    }

    // outstandingQ & pendingQ should be cleared.
    auto outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 0);
    EXPECT_EQ(globalPendingIoRetryQ.size(), 0);

    // Issue write => Add a request to the range map (outstanding map)
    MetaFsIoReqMsg req1;
    {
        req1.fd = fd;
        req1.ioMode = MetaIoModeEnum::Async;
        req1.reqType = MetaIoReqTypeEnum::Write;
        req1.targetMediaType = MetaStorageType::SSD;
        req1.byteOffsetInFile = 0;
        req1.byteSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
        req1.isFullFileIo = false;
        req1.SetValidForIoExecution();
        req1.tagId = aiocbTagIdAllocator();

        // push req1 to range map
        globalIoRangeOverlapChker.PushReqToRangeLockMap(&req1);
    }

    outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 1);

    // Issue read : W -> R conflicted
    MetaFsIoReqMsg req2;
    { // the request range is overlap to the previous I/O
        req2.fd = fd;
        req2.ioMode = MetaIoModeEnum::Async;
        req2.reqType = MetaIoReqTypeEnum::Read;
        req2.targetMediaType = MetaStorageType::SSD;
        req2.byteOffsetInFile = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE - 10;
        req2.byteSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
        req2.isFullFileIo = false;
        req2.tagId = aiocbTagIdAllocator();

        // Check LPN overlap between req1 and req2
        EXPECT_EQ(globalIoRangeOverlapChker.IsRangeOverlapConflicted(&req2), true);

        // push req2 message to retry Q
        globalPendingIoRetryQ.push_back(&req2);
    }
    EXPECT_EQ(globalPendingIoRetryQ.size(), 1);

    // free req1 from range map
    globalIoRangeOverlapChker.FreeLockContext(req1.tagId, false);
    outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 0);

    // Issue read, buf there is no outstaing I/O => no conflicted
    MetaFsIoReqMsg req3;
    {
        req3.fd = fd;
        req3.ioMode = MetaIoModeEnum::Async;
        req3.reqType = MetaIoReqTypeEnum::Read;
        req3.targetMediaType = MetaStorageType::SSD;
        req3.byteOffsetInFile = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE - 10;
        req3.byteSize = 40;
        req3.tagId = aiocbTagIdAllocator();

        EXPECT_EQ(globalIoRangeOverlapChker.IsRangeOverlapConflicted(&req3), false);

        // push req3 to range map
        globalIoRangeOverlapChker.PushReqToRangeLockMap(&req3);
    }

    // three are 2 remaing eqs: req2 (retryQ) , req 3 (outstandingQ)
    outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 1);

    // free req3 from outstandingQ
    while (0 != (outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount()))
    {
        globalIoRangeOverlapChker.FreeLockContext(globalIoRangeOverlapChker.GetOutstandingMioMap()->FindFirstSet(0), false);
    }

    outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 0);

    // fetch read from reqty Q
    int issuedMioCnt = DiscoverIORangeOverlap();
    EXPECT_EQ(issuedMioCnt, 1);
}

/* Case 2: 
          (0)             (chunk size)
[Req1 R]   ------------------ 
[Req2 R]                  -------       

Req2 R : R - R case 
*/

TEST_F(UtMIMRangeLockChker, RangeOverlapChkTest2)
{
    FileFDType fd;
    const FileFDType invalidFD = MetaFsCommonConst::INVALID_FD;

    fd = OpenDummyFile();
    if (fd == invalidFD)
    {
        FileSizeType fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100; // 400KB
        fd = CreateFileAndOpen(fileSize);
    }

    // outstandingQ & pendingQ should be cleared.
    auto outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 0);
    EXPECT_EQ(globalPendingIoRetryQ.size(), 0);

    // Issue write => Add a request to the range map (outstanding map)
    MetaFsIoReqMsg req1;
    {
        req1.fd = fd;
        req1.ioMode = MetaIoModeEnum::Async;
        req1.reqType = MetaIoReqTypeEnum::Read;
        req1.targetMediaType = MetaStorageType::SSD;
        req1.byteOffsetInFile = 0;
        req1.byteSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
        req1.isFullFileIo = false;
        req1.SetValidForIoExecution();
        req1.tagId = aiocbTagIdAllocator();

        // push req1 to range map
        globalIoRangeOverlapChker.PushReqToRangeLockMap(&req1);
    }
    outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 1);

    // Issue read : R -> R no need to check conflict
    MetaFsIoReqMsg req2;
    {
        req2.fd = fd;
        req2.ioMode = MetaIoModeEnum::Async;
        req2.reqType = MetaIoReqTypeEnum::Read;
        req2.targetMediaType = MetaStorageType::SSD;
        req2.byteOffsetInFile = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE - 10;
        req2.byteSize = 40;
        req2.tagId = aiocbTagIdAllocator();

        EXPECT_EQ(globalIoRangeOverlapChker.IsRangeOverlapConflicted(&req2), false);

        // push req2 to range map
        globalIoRangeOverlapChker.PushReqToRangeLockMap(&req2);
    }
    outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 2);

    // there are 2 remaing reqs: req 1/3 (outstandingQ)
    while (0 != (outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount()))
    {
        globalIoRangeOverlapChker.FreeLockContext(globalIoRangeOverlapChker.GetOutstandingMioMap()->FindFirstSet(0), false);
    }

    outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 0);
}

/* Case 3: 
          (0)             (chunk size)
[Req1 W]   ------------------ 
[Req2 W]                  -------       
[Req3 R]      -------       

Req2 W : LPN overlapped (req1 vs. req2), and put to retryQ
Req3 R : LPN overlapped (req1 vs. req3), and put to retyQ
Req1 W release from outstanding Q
*/

TEST_F(UtMIMRangeLockChker, RangeOverlapChkTest3)
{
    FileFDType fd;
    const FileFDType invalidFD = MetaFsCommonConst::INVALID_FD;

    fd = OpenDummyFile();
    if (fd == invalidFD)
    {
        FileSizeType fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100; // 400KB
        fd = CreateFileAndOpen(fileSize);
    }

    // outstandingQ & pendingQ should be cleared.
    auto outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 0);
    EXPECT_EQ(globalPendingIoRetryQ.size(), 0);

    // Issue write => Add a request to the range map (outstanding map)
    MetaFsIoReqMsg req1;
    {
        req1.fd = fd;
        req1.ioMode = MetaIoModeEnum::Async;
        req1.reqType = MetaIoReqTypeEnum::Write;
        req1.targetMediaType = MetaStorageType::SSD;
        req1.byteOffsetInFile = 0;
        req1.byteSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
        req1.isFullFileIo = false;
        req1.SetValidForIoExecution();
        req1.tagId = aiocbTagIdAllocator();

        // push req1 to range map
        globalIoRangeOverlapChker.PushReqToRangeLockMap(&req1);
    }
    outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 1);

    // Issue write : LPN conflcted between Req1 and Req2
    MetaFsIoReqMsg req2;
    {
        req2.fd = fd;
        req2.ioMode = MetaIoModeEnum::Async;
        req2.reqType = MetaIoReqTypeEnum::Write;
        req2.targetMediaType = MetaStorageType::SSD;
        req2.byteOffsetInFile = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE - 10;
        req2.byteSize = 40;
        req2.tagId = aiocbTagIdAllocator();

        EXPECT_EQ(globalIoRangeOverlapChker.IsRangeOverlapConflicted(&req2), true);

        // push req2 message to retry Q
        globalPendingIoRetryQ.push_back(&req2);
    }
    EXPECT_EQ(globalPendingIoRetryQ.size(), 1);

    // Issue Read : LPN Conflicted between Req 1 and Req3 since Req1 is still alive in the outstandingQ.
    MetaFsIoReqMsg req3;
    {
        req3.fd = fd;
        req3.ioMode = MetaIoModeEnum::Async;
        req3.reqType = MetaIoReqTypeEnum::Read;
        req3.targetMediaType = MetaStorageType::SSD;
        req3.byteOffsetInFile = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE - 1024;
        req3.byteSize = 40;
        req3.tagId = aiocbTagIdAllocator();

        EXPECT_EQ(globalIoRangeOverlapChker.IsRangeOverlapConflicted(&req3), true);

        // push req2 message to retry Q
        globalPendingIoRetryQ.push_back(&req3);
    }

    EXPECT_EQ(globalPendingIoRetryQ.size(), 2);

    // free req1 from range map
    globalIoRangeOverlapChker.FreeLockContext(req1.tagId, true);
    outstandingQCnt = globalIoRangeOverlapChker.GetOutstandingMioCount();
    EXPECT_EQ(outstandingQCnt, 0);

    // there are only 2 I/Os in retryQ
    int issuedMioCnt = DiscoverIORangeOverlap();
    EXPECT_EQ(issuedMioCnt, 2);
}

int
UtMIMRangeLockChker::DiscoverIORangeOverlap(void)
{
    int issuedMioCnt = 0;

    for (auto it = globalPendingIoRetryQ.begin(); it != globalPendingIoRetryQ.end();)
    {
        MetaFsIoReqMsg* pendingIoReq = *it;

        if (!globalIoRangeOverlapChker.IsRangeOverlapConflicted(pendingIoReq))
        {
            MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[Dispatch next candidate!!] ioreq.id={}", pendingIoReq->tagId);

            globalPendingIoRetryQ.erase(it++);
            issuedMioCnt++;
        }
        else
        {
            ++it;
        }
    }

    return issuedMioCnt;
}

FileFDType
UtMIMRangeLockChker::CreateFileAndOpen(FileSizeType fileSize)
{
    FileFDType fd;
    CreateDummyFile(fileSize);
    fd = OpenDummyFile();
    MetaLpnType fileBaseLpn;
    {
        IBOF_EVENT_ID sc;
        sc = mvmTopMgr.GetFileBaseLpn(fd, fileBaseLpn);
        assert(sc == IBOF_EVENT_ID::SUCCESS);
    }

    return fd;
}
