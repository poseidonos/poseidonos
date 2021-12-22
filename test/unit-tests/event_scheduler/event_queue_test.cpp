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

#include "src/event_scheduler/event_queue.h"

#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_manager_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{

class StubEventEQ : public Event
{
public:
    StubEventEQ(bool isFrontEndEvent = false, BackendEvent event = BackendEvent_Unknown,
        AffinityManager* affinityManager = nullptr)
    : Event(isFrontEndEvent, event, affinityManager)
    {
    }
    virtual bool Execute(void) final
    {
        return true;
    }
};

TEST(EventQueue, EventQueue_Stack)
{
    // Given: Do nothing

    // When: Create EventQueue
    EventQueue eventQueue;

    // Then: Do nothing
}

TEST(EventQueue, EventQueue_Heap)
{
    // Given: Do nothing

    // When: Create EventQueue
    EventQueue* eventQueue = new EventQueue;
    delete eventQueue;

    // Then: Do nothing
}

TEST(EventQueue, EnqueueEvent_InputNullptr)
{
    // Given: EventQueue
    EventQueue eventQueue;
    uint32_t actual, expected = 0;

    // When: Call EnqueueEvent with nullptr
    eventQueue.EnqueueEvent(nullptr);
    actual = eventQueue.GetQueueSize();

    // Then: Expect to return 0
    EXPECT_EQ(actual, expected);
}

TEST(EventQueue, EnqueueEvent_InputNormal)
{
    // Given: EventQueue, EventSmartPtr
    EventQueue eventQueue;
    auto input = std::make_shared<StubEventEQ>(false);
    uint32_t actual, expected = 1;

    // When: Call EnqueueEvent with nullptr
    eventQueue.EnqueueEvent(input);
    actual = eventQueue.GetQueueSize();

    // Then: Expect to return 0
    EXPECT_EQ(actual, expected);
}

TEST(EventQueue, DequeueEvent_EmptyQueue)
{
    // Given: MockQosManager, EventQueue
    NiceMock<MockQosManager> mockQosManager;
    EventQueue eventQueue {&mockQosManager};
    EventSmartPtr event;

    // When: Call DequeueEvent with empty Queue
    event = eventQueue.DequeueEvent();

    // Then: Expect to return nullptr
    EXPECT_EQ(nullptr, event);
}

TEST(EventQueue, DequeueEvent_TwoTypes)
{
    // Given: MockQosManager, EventQueue
    NiceMock<MockQosManager> mockQosManager;
    EventQueue eventQueue {&mockQosManager};
    auto eventNotFrontend = std::make_shared<StubEventEQ>(false);
    auto eventFrontend = std::make_shared<StubEventEQ>(true);
    EventSmartPtr event;

    // When 1: Call DequeueEvent
    eventQueue.EnqueueEvent(eventNotFrontend);
    event = eventQueue.DequeueEvent();

    // Then 1: Expect to return eventNotFrontend
    EXPECT_EQ(eventNotFrontend.get(), event.get());

    // When 2: Call DequeueEvent
    eventQueue.EnqueueEvent(eventFrontend);
    event = eventQueue.DequeueEvent();

    // Then 2: Expect to return eventFrontend
    EXPECT_EQ(eventFrontend.get(), event.get());
}

TEST(EventQueue, GetQueueSize_EnqTwice_DeqOnce)
{
    // Given: EventQueue, EventSmartPtr, MockQosManager, Enqueue twice, Dequeue once
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IsFeQosEnabled()).WillByDefault(Return(false));
    auto input1 = std::make_shared<StubEventEQ>(false);
    auto input2 = std::make_shared<StubEventEQ>(true);
    EventQueue eventQueue {&mockQosManager};
    eventQueue.EnqueueEvent(input1);
    eventQueue.EnqueueEvent(input2);
    eventQueue.DequeueEvent();
    uint32_t actual, expected = 1;

    // When: Call GetQueueSize
    actual = eventQueue.GetQueueSize();

    // Then: Expect to return 1
    EXPECT_EQ(actual, expected);
}

} // namespace pos
