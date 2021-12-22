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

#include "src/event_scheduler/scheduler_queue.h"

#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_manager_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{

class StubEventSQ : public Event
{
public:
    StubEventSQ(bool isFrontEndEvent = false, BackendEvent event = BackendEvent_Unknown,
        AffinityManager* affinityManager = nullptr)
    : Event(isFrontEndEvent, event, affinityManager)
    {
    }
    virtual bool Execute(void) final
    {
        return true;
    }
};

TEST(SchedulerQueue, SchedulerQueue_Stack)
{
    // Given: Do nothing

    // When: Create SchedulerQueue
    SchedulerQueue schedulerQueue;

    // Then: Do nothing
}

TEST(SchedulerQueue, SchedulerQueue_Heap)
{
    // Given: Do nothing

    // When: Create SchedulerQueue
    SchedulerQueue* schedulerQueue = new SchedulerQueue;
    delete schedulerQueue;

    // Then: Do nothing
}

TEST(SchedulerQueue, EnqueueEvent_NullEvent)
{
    // Given: SchedulerQueue
    SchedulerQueue schedulerQueue;

    // When: Call EnqueueEvent with nullptr
    schedulerQueue.EnqueueEvent(nullptr);

    // Then: Do nothing
}

TEST(SchedulerQueue, EnqueueEvent_StubEvent)
{
    // Given: SchedulerQueue, MockQosManager, StubEventSQ
    NiceMock<MockQosManager> mockQosManager;
    SchedulerQueue schedulerQueue {&mockQosManager};
    auto eventFrontend = std::make_shared<StubEventSQ>(true);
    auto eventNotFrontend = std::make_shared<StubEventSQ>(false);

    // When: Call EnqueueEvent with StubEventSQ
    schedulerQueue.EnqueueEvent(eventFrontend);
    schedulerQueue.EnqueueEvent(eventNotFrontend);

    // Then: Do nothing
}

TEST(SchedulerQueue, DequeueEvent_Empty)
{
    // Given: SchedulerQueue, MockQosManager
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IsFeQosEnabled()).WillByDefault(Return(false));
    SchedulerQueue schedulerQueue {&mockQosManager};
    EventSmartPtr event;

    // When: Call DequeueEvent with empty queue
    event = schedulerQueue.DequeueEvent();

    // Then: Expect to return nullptr
    EXPECT_EQ(nullptr, event);
}

TEST(SchedulerQueue, DequeueEvent_OneInput)
{
    // Given: SchedulerQueue, MockQosManager, StubEventSQ
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IsFeQosEnabled()).WillByDefault(Return(false));
    SchedulerQueue schedulerQueue {&mockQosManager};
    auto event = std::make_shared<StubEventSQ>(false);
    schedulerQueue.EnqueueEvent(event);
    EventSmartPtr result;

    // When: Call DequeueEvent with empty queue
    result = schedulerQueue.DequeueEvent();

    // Then: Expect to return nullptr
    EXPECT_EQ(event.get(), result.get());
}

} // namespace pos
