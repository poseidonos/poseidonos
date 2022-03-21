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

#include "src/event_scheduler/event_worker.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class StubEventEW : public Event
{
public:
    explicit StubEventEW(bool result)
    : Event()
    {
        resultExecute = result;
    }
    virtual bool Execute(void) final
    {
        return resultExecute;
    }

private:
    bool resultExecute;
};

TEST(EventWorker, EventWorker_Stack)
{
    // Given: cpu_set_t
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);

    // When: Create EventWorker
    NiceMock<MockEventScheduler> mockEventScheduler;
    EventWorker eventWorker{cpuSet, &mockEventScheduler, 1};
    // Then: Do nothing
}

TEST(EventWorker, EventWorker_Heap)
{
    // Given: cpu_set_t
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);

    // When: Create EventWorker
    NiceMock<MockEventScheduler> mockEventScheduler;
    EventWorker* eventWorker = new EventWorker {cpuSet, &mockEventScheduler, 1};
    delete eventWorker;

    // Then: Do nothing
}

TEST(EventWorker, EnqueueEvent_And_GetQueueSize_SimpleCall)
{
    // Given: cpu_set_t, EventWorker, StubEvent
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);

    auto event = std::make_shared<StubEventEW>(true);

    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, CheckAndSetQueueOccupancy(_)).WillByDefault(Return());
    ON_CALL(mockEventScheduler, PickWorkerEvent(_)).WillByDefault(Return(event));
    EventWorker eventWorker{cpuSet, &mockEventScheduler, 1};
    uint32_t actual, expected = 1;

    // When: Call EnqueueEvent
    eventWorker.EnqueueEvent(event);
    actual = eventWorker.GetQueueSize();

    // Then: Expect to return 1
    EXPECT_EQ(actual, expected);
}

TEST(DISABLED_EventWorker, Run_TwoEvents)
{
    // Given: cpu_set_t, EventWorker, StubEvent, MockEventScheduler
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    auto eventTrue = std::make_shared<StubEventEW>(true);
    auto eventFalse = std::make_shared<StubEventEW>(false);
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, CheckAndSetQueueOccupancy(_)).WillByDefault(Return());
    EXPECT_CALL(mockEventScheduler, PickWorkerEvent(_)).WillOnce(Return(eventTrue)).WillOnce(Return(eventFalse)).WillRepeatedly(Return(nullptr));

    EventWorker eventWorker{cpuSet, &mockEventScheduler, 1};
    eventWorker.EnqueueEvent(eventTrue);
    eventWorker.EnqueueEvent(eventFalse);

    // When, Then: Wait until Run loop handles all event in the queue
    while (0 != eventWorker.GetQueueSize())
    {
        usleep(10);
    }
}

} // namespace pos
