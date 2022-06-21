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

#include "src/mapper/map/create_map_flush_event.h"

#include <gtest/gtest.h>

#include <memory>

#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/mapper/map/sequential_page_finder_mock.h"

using ::testing::_;
using ::testing::Return;

namespace pos
{
TEST(CreateMapFlushEvent, Execute_testIfTheEventCanCreateSingleEvent)
{
    // given
    MockEventScheduler scheduler;
    std::unique_ptr<MockSequentialPageFinder> sequentialPages = std::make_unique<MockSequentialPageFinder>();
    MpageSet mpageSet;

    // when
    EXPECT_CALL(*sequentialPages.get(), IsRemaining)
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*sequentialPages.get(), PopNextMpageSet).WillOnce(Return(mpageSet));
    std::unique_ptr<CreateMapFlushInfo> info = std::make_unique<CreateMapFlushInfo>(nullptr, nullptr, nullptr, nullptr, std::move(sequentialPages));
    CreateMapFlushEvent event(std::move(info), &scheduler);

    // then
    EXPECT_CALL(scheduler, EnqueueEvent).WillOnce(Return());
    EXPECT_TRUE(event.Execute());
}

TEST(CreateMapFlushEvent, Execute_testIfTheEventCanCreateThreeEvents)
{
    // given
    MockEventScheduler scheduler;
    std::unique_ptr<MockSequentialPageFinder> sequentialPages = std::make_unique<MockSequentialPageFinder>();
    MpageSet mpageSet;

    // when
    EXPECT_CALL(*sequentialPages.get(), IsRemaining)
        .Times(4)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*sequentialPages.get(), PopNextMpageSet).Times(3).WillRepeatedly(Return(mpageSet));
    std::unique_ptr<CreateMapFlushInfo> info = std::make_unique<CreateMapFlushInfo>(nullptr, nullptr, nullptr, nullptr, std::move(sequentialPages));
    CreateMapFlushEvent event(std::move(info), &scheduler);

    // then
    EXPECT_CALL(scheduler, EnqueueEvent).Times(3).WillRepeatedly(Return());
    EXPECT_TRUE(event.Execute());
}
} // namespace pos
