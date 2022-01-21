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
TEST(MetaFsIoMultilevelQ, Normal)
{
    const uint32_t REQ_COUNT = 10;
    MetaFsIoMultilevelQ<MetaFsIoRequest*, IoRequestPriority>* multiQ = new MetaFsIoMultilevelQ<MetaFsIoRequest*, IoRequestPriority>();
    std::array<std::unordered_set<MetaFsIoRequest*>, (size_t)IoRequestPriority::MAX> requests;

    // push
    for (uint32_t req = 0; req < REQ_COUNT; req++)
    {
        MetaFsIoRequest* reqH = new MetaFsIoRequest();
        MetaFsIoRequest* reqL = new MetaFsIoRequest();

        reqH->priority = IoRequestPriority::Highest;
        reqL->priority = IoRequestPriority::Normal;

        multiQ->Enqueue(reqH, IoRequestPriority::Highest);
        multiQ->Enqueue(reqL, IoRequestPriority::Normal);

        requests[(size_t)IoRequestPriority::Highest].insert(reqH);
        requests[(size_t)IoRequestPriority::Normal].insert(reqL);
    }

    // pop
    uint32_t total_count = 0;
    while (total_count < (REQ_COUNT * 2))
    {
        MetaFsIoRequest* reqMsg = multiQ->Dequeue();
        if (nullptr == reqMsg)
            continue;

        EXPECT_EQ(requests[(size_t)(reqMsg->priority)].count(reqMsg), 1);
        requests[(size_t)(reqMsg->priority)].erase(reqMsg);
        total_count++;

        delete reqMsg;
    }

    EXPECT_EQ(multiQ->Dequeue(), nullptr);
    EXPECT_EQ(total_count, REQ_COUNT * 2);

    delete multiQ;
}
} // namespace pos
