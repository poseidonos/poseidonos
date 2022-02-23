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

#include "src/metafs/mim/metafs_io_multilevel_q.h"

#include <gtest/gtest.h>

#include <unordered_set>
#include <array>

#include "src/metafs/mim/metafs_io_request.h"

namespace pos
{
TEST(MetaFsIoMultilevelQ, testIfDequeueCountMatchesWithEnqueueCountWithTwoDifferentPriorities)
{
    const uint32_t REQ_COUNT = 10;
    MetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>* multiQ = new MetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>();
    std::array<std::unordered_set<MetaFsIoRequest*>, (size_t)RequestPriority::MAX> requests;

    ASSERT_EQ(multiQ->Dequeue(), nullptr);

    // push
    for (uint32_t req = 0; req < REQ_COUNT; req++)
    {
        MetaFsIoRequest* reqH = new MetaFsIoRequest();
        MetaFsIoRequest* reqL = new MetaFsIoRequest();

        reqH->priority = RequestPriority::Highest;
        reqL->priority = RequestPriority::Normal;

        multiQ->Enqueue(reqH, RequestPriority::Highest);
        multiQ->Enqueue(reqL, RequestPriority::Normal);

        requests[(size_t)RequestPriority::Highest].insert(reqH);
        requests[(size_t)RequestPriority::Normal].insert(reqL);
    }

    // pop
    uint32_t total_count = 0;
    while (total_count < (REQ_COUNT * 2))
    {
        MetaFsIoRequest* reqMsg = multiQ->Dequeue();
        if (nullptr == reqMsg)
            continue;

        EXPECT_NE(requests[(size_t)(reqMsg->priority)].find(reqMsg), requests[(size_t)(reqMsg->priority)].end());
        requests[(size_t)(reqMsg->priority)].erase(reqMsg);
        total_count++;

        delete reqMsg;
    }

    EXPECT_EQ(total_count, REQ_COUNT * 2);
    EXPECT_EQ(0, requests[(size_t)RequestPriority::Highest].size());
    EXPECT_EQ(0, requests[(size_t)RequestPriority::Normal].size());

    delete multiQ;
}

class TestMetaFsRequest : public MetaFsIoRequest
{
public:
    TestMetaFsRequest(std::unordered_set<TestMetaFsRequest*>* v) : v(v)
    {
    }
    ~TestMetaFsRequest(void)
    {
        v->erase(this);
    }

private:
    std::unordered_set<TestMetaFsRequest*>* v;
};

TEST(MetaFsIoMultilevelQ, destructor_testIfTheDesctructorWillDeleteAllTheItemsInTheQueues)
{
    const uint32_t REQ_COUNT = 10;
    MetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>* multiQ = new MetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>();
    std::array<std::unordered_set<TestMetaFsRequest*>, (size_t)RequestPriority::MAX> requests;

    ASSERT_EQ(multiQ->Dequeue(), nullptr);

    // push
    for (uint32_t req = 0; req < REQ_COUNT; req++)
    {
        TestMetaFsRequest* reqH = new TestMetaFsRequest(&requests[(size_t)RequestPriority::Highest]);
        TestMetaFsRequest* reqL = new TestMetaFsRequest(&requests[(size_t)RequestPriority::Normal]);

        reqH->priority = RequestPriority::Highest;
        reqL->priority = RequestPriority::Normal;

        multiQ->Enqueue(reqH, RequestPriority::Highest);
        multiQ->Enqueue(reqL, RequestPriority::Normal);

        requests[(size_t)RequestPriority::Highest].insert(reqH);
        requests[(size_t)RequestPriority::Normal].insert(reqL);
    }

    EXPECT_EQ(REQ_COUNT, requests[(size_t)RequestPriority::Highest].size());
    EXPECT_EQ(REQ_COUNT, requests[(size_t)RequestPriority::Normal].size());

    // desctuct
    delete multiQ;

    // check
    EXPECT_EQ(0, requests[(size_t)RequestPriority::Highest].size());
    EXPECT_EQ(0, requests[(size_t)RequestPriority::Normal].size());
}
} // namespace pos
