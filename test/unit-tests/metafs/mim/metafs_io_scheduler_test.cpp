/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/metafs/mim/metafs_io_scheduler.h"

#include <gtest/gtest.h>

#include <cstring>
#include <map>

#include "src/metafs/mim/metafs_io_request.h"
#include "src/metafs/mim/scalable_meta_io_worker.h"
#include "test/unit-tests/metafs/lib/metafs_time_interval.h"

using ::testing::Return;
using ::testing::Test;

namespace pos
{
class ScalableMetaIoWorkerTester : public ScalableMetaIoWorker
{
public:
    ScalableMetaIoWorkerTester(void)
    {
    }
    virtual void EnqueueNewReq(MetaFsIoRequest* req)
    {
        countMap.insert({req->baseMetaLpn, req->byteSize});
    }
    void ClearCount(void)
    {
        countMap.clear();
    }
    size_t GetRequestedCount(void)
    {
        return countMap.size();
    }
    size_t GetRequestedSize(void)
    {
        size_t size = 0;
        for (auto& lpnInfo : countMap)
        {
            size += lpnInfo.second;
        }
        return size;
    }
    MetaLpnType GetTheFirstLpn(void)
    {
        return countMap.begin()->first;
    }
    MetaLpnType GetTheLastLpn(void)
    {
        return (countMap.rbegin())->first;
    }

private:
    std::map<MetaLpnType, size_t> countMap;
};

class MetaFsIoSchedulerTexture : public ::testing::Test
{
public:
    MetaFsIoSchedulerTexture(void)
    : interval(nullptr),
      scheduler(nullptr)
    {
    }

    virtual ~MetaFsIoSchedulerTexture(void)
    {
    }

    virtual void SetUp(void)
    {
        const std::vector<int> weight = {1, 1, 1, 1};
        interval = new MockMetaFsTimeInterval(1000);
        ON_CALL(*interval, CheckInterval).WillByDefault(Return(false));
        CPU_SET(0, &mioCoreSet);
        scheduler = new MetaFsIoScheduler(0, 0, 1, "Test", mioCoreSet, nullptr, nullptr, interval, weight, false);
        scheduler->RegisterMetaIoWorkerForTest(&metaIoWorker);
    }

    virtual void TearDown(void)
    {
        delete scheduler;
    }

protected:
    MockMetaFsTimeInterval* interval;
    ScalableMetaIoWorkerTester metaIoWorker;
    cpu_set_t mioCoreSet;
    MetaFsIoScheduler* scheduler;
};

TEST_F(MetaFsIoSchedulerTexture, IssueRequest_testIfAnAlignedRequestCreatesOneRequest)
{
    // given
    MetaFileExtent extents;
    extents.SetStartLpn(0);
    extents.SetCount(1024);
    MetaFileContext fileCtx;
    fileCtx.chunkSize = 4032;
    fileCtx.fileBaseLpn = 0;
    fileCtx.extentsCount = 1;
    fileCtx.CopyExtentsFrom(&extents, fileCtx.extentsCount);
    MetaFsIoRequest originReq;
    MetaFsIoRequest* req;

    // when
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 0;
    req->byteSize = 4032;
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 1);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 1);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 0);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 0);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032);

    // when
    metaIoWorker.ClearCount();
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032;
    req->byteSize = 4032;
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 1);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 1);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 1);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 1);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032);

    // when
    metaIoWorker.ClearCount();
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032;
    req->byteSize = 2016;
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 1);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 1);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 1);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 1);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 2016);
}

TEST_F(MetaFsIoSchedulerTexture, IssueRequest_testIfAnUnalignedRequestCreatesTwoRequests)
{
    // given
    MetaFileExtent extents;
    extents.SetStartLpn(0);
    extents.SetCount(1024);
    MetaFileContext fileCtx;
    fileCtx.chunkSize = 4032;
    fileCtx.fileBaseLpn = 0;
    fileCtx.extentsCount = 1;
    fileCtx.CopyExtentsFrom(&extents, fileCtx.extentsCount);
    MetaFsIoRequest originReq;
    MetaFsIoRequest* req;

    // when
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 0;
    req->byteSize = 4032 + 2016;
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 2);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 2);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 0);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 1);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032 + 2016);

    // when
    metaIoWorker.ClearCount();
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 2016;
    req->byteSize = 4032;
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 2);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 2);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 0);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 1);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032);

    // when
    metaIoWorker.ClearCount();
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032;
    req->byteSize = 4032 + 2016;
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 2);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 2);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 1);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 2);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032 + 2016);
}

TEST_F(MetaFsIoSchedulerTexture, IssueRequest_testIfABigAlignedRequestCreatesManyRequests)
{
    // given
    MetaFileExtent extents;
    extents.SetStartLpn(43082495);
    extents.SetCount(2048);
    MetaFileContext fileCtx;
    fileCtx.chunkSize = 4032;
    fileCtx.fileBaseLpn = 43082495;
    fileCtx.extentsCount = 1;
    fileCtx.CopyExtentsFrom(&extents, fileCtx.extentsCount);
    MetaFsIoRequest originReq;
    MetaFsIoRequest* req;

    // when
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 0;
    req->byteSize = 4032 * 1048; // 1048 lpns
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 1048);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 1048);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 43082495);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 43082495 + 1048 - 1);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032 * 1048);
}

TEST_F(MetaFsIoSchedulerTexture, IssueRequest_testForProcessingRequestsForFilesWithMultiExtentSimple)
{
    // given
    MetaFileExtent extents[3];
    extents[0].SetStartLpn(5);
    extents[0].SetCount(5);
    extents[1].SetStartLpn(20);
    extents[1].SetCount(10);
    extents[2].SetStartLpn(40);
    extents[2].SetCount(5);
    MetaFileContext fileCtx;
    fileCtx.chunkSize = 4032;
    fileCtx.fileBaseLpn = 5;
    fileCtx.extentsCount = 3;
    fileCtx.CopyExtentsFrom(extents, fileCtx.extentsCount);
    MetaFsIoRequest originReq;
    MetaFsIoRequest* req;

    // when, 1st
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032;
    req->byteSize = 4032; // lpn 6
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 1);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 1);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 6);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 6);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032);

    // when, 2nd
    metaIoWorker.ClearCount();
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032 * 2;
    req->byteSize = 4032 * 6; // lpn 7,8,9,20,21,22
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 6);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 6);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 7);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 22);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032 * 6);

    // when, 3rd
    metaIoWorker.ClearCount();
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032 * 4;
    req->byteSize = 4032 * 12; // lpn 9,20...29,40
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 12);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 12);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 9);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 40);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032 * 12);
}

TEST_F(MetaFsIoSchedulerTexture, IssueRequest_testForProcessingRequestsForFilesWithMultiExtent)
{
    // given
    MetaFileExtent extents[3];
    extents[0].SetStartLpn(43082495);
    extents[0].SetCount(1048);
    extents[1].SetStartLpn(43085111);
    extents[1].SetCount(2088);
    extents[2].SetStartLpn(43089807);
    extents[2].SetCount(512);
    MetaFileContext fileCtx;
    fileCtx.chunkSize = 4032;
    fileCtx.fileBaseLpn = 43082495;
    fileCtx.extentsCount = 3;
    fileCtx.CopyExtentsFrom(extents, fileCtx.extentsCount);
    MetaFsIoRequest originReq;
    MetaFsIoRequest* req;

    // when, 1st
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032; // from 2nd lpn (43082495 + 1)
    req->byteSize = 4032;         // to 2nd lpn (43082495 + 1)
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 1);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 1);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 43082495 + 1);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 43082495 + 1);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4032);

    // when, 2nd
    metaIoWorker.ClearCount();
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032 * 2; // from 3rd lpn (43082495 + 2)
    req->byteSize = 4032 * 1024;      // to 1026th lpn (43082495 + 1025)
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 4128768 / 4032);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 4128768 / 4032);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 43082495 + 2);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 43082495 + 2 + 1024 - 1);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4128768);

    // when, 3rd
    metaIoWorker.ClearCount();
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032 * 1026; // from 1027th lpn (43082495 + 1026)
    req->byteSize = 4032 * 1024;         // to 1002nd of next extent (43085111 + 1001)
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 4128768 / 4032);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 4128768 / 4032);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 43083521);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 43085111 + 1001);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4128768);

    // when, 4th
    metaIoWorker.ClearCount();
    req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;
    req->originalMsg = &originReq;
    req->byteOffsetInFile = 4032 * 2050; // from 1003rd lpn of 2nd extent (43085111 + 1002)
    req->byteSize = 4032 * 1024;         // to 2026th of the extent (43085111 + 2025)
    req->targetMediaType = MetaStorageType::SSD;

    // then
    scheduler->IssueRequestAndDelete(req);
    EXPECT_EQ(originReq.requestCount, 4128768 / 4032);
    EXPECT_EQ(metaIoWorker.GetRequestedCount(), 4128768 / 4032);
    EXPECT_EQ(metaIoWorker.GetTheFirstLpn(), 43085111 + 1002);
    EXPECT_EQ(metaIoWorker.GetTheLastLpn(), 43085111 + 2025);
    EXPECT_EQ(metaIoWorker.GetRequestedSize(), 4128768);

    // then, check for metric
    size_t arraySize = 0;
    const int64_t* countArray = scheduler->GetIssueCount(arraySize);

    EXPECT_EQ(arraySize, (size_t)MetaStorageType::Max);
    EXPECT_EQ(countArray[(size_t)MetaStorageType::SSD], 1 + 1024 + 1024 + 1024);
    EXPECT_EQ(countArray[(size_t)MetaStorageType::NVRAM], 0);
    EXPECT_EQ(countArray[(size_t)MetaStorageType::JOURNAL_SSD], 0);
}
} // namespace pos
