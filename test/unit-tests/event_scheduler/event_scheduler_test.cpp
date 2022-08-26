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

#include "src/event_scheduler/event_scheduler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/cpu_affinity/affinity_manager.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_queue.h"
#include "src/event_scheduler/event_worker.h"
#include "src/event_scheduler/minimum_job_policy.h"
#include "src/event_scheduler/scheduler_queue.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/qos/qos_spdk_manager.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class StubEventES : public Event
{
public:
    StubEventES(bool isFrontEndEvent = false, BackendEvent event = BackendEvent_Unknown,
        AffinityManager* affinityManager = nullptr)
    : Event(isFrontEndEvent, event, affinityManager)
    {
    }
    virtual bool Execute(void) final
    {
        return true;
    }
};

TEST(EventScheduler, EventScheduler_Stack)
{
    // Given: MockQosManager, MockConfigManager, MockAffinityManager
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockConfigManager> mockConfigManager;
    NiceMock<MockAffinityManager> mockAffinityManager;

    // When 1: Create EventScheduler with numaDedicatedSchedulingPolicy
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(static_cast<int>(EID(SUCCESS))));
    EventScheduler eventScheduler1{&mockQosManager, &mockConfigManager, &mockAffinityManager};

    // When 2: Create EventScheduler without numaDedicatedSchedulingPolicy
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(static_cast<int>(EID(EVENT_ID_MAPPING_WRONG))));
    EventScheduler eventScheduler2{&mockQosManager, &mockConfigManager, &mockAffinityManager};

    // Then: Do nothing
}

TEST(EventScheduler, EventScheduler_Heap)
{
    // Given: Do nothing

    // When: Create EventScheduler
    EventScheduler* eventScheduler = new EventScheduler;
    delete eventScheduler;

    // Then: Do nothing
}

TEST(EventScheduler, Initialize_EnoughCore)
{
    // Given: MockQosManager, MockConfigManager, MockAffinityManager, EventScheduler
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockConfigManager> mockConfigManager;
    NiceMock<MockAffinityManager> mockAffinityManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(static_cast<int>(EID(SUCCESS))));
    ON_CALL(mockAffinityManager, GetTotalCore()).WillByDefault(Return(10));
    ON_CALL(mockAffinityManager, GetNumaIdFromCoreId(_)).WillByDefault(Return(0));
    EventScheduler eventSchechduler{&mockQosManager, &mockConfigManager, &mockAffinityManager};
    cpu_set_t schedulerCPU, eventCPU;
    CPU_ZERO(&schedulerCPU);
    CPU_SET(1, &schedulerCPU);
    CPU_ZERO(&eventCPU);
    CPU_SET(2, &eventCPU);

    // When: Call Initialize
    eventSchechduler.Initialize(1, schedulerCPU, eventCPU);

    // Then: Do nothing
}

TEST(EventScheduler, Initialize_NotEnoughCore)
{
    // Given: MockQosManager, MockConfigManager, MockAffinityManager, EventScheduler
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockConfigManager> mockConfigManager;
    NiceMock<MockAffinityManager> mockAffinityManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(static_cast<int>(EID(SUCCESS))));
    ON_CALL(mockAffinityManager, GetTotalCore()).WillByDefault(Return(0));
    ON_CALL(mockAffinityManager, GetNumaIdFromCoreId(_)).WillByDefault(Return(0));
    EventScheduler eventSchechduler{&mockQosManager, &mockConfigManager, &mockAffinityManager};
    cpu_set_t schedulerCPU, eventCPU;
    CPU_ZERO(&schedulerCPU);
    CPU_SET(1, &schedulerCPU);
    CPU_ZERO(&eventCPU);
    CPU_SET(2, &eventCPU);

    // When: Call Initialize
    eventSchechduler.Initialize(1, schedulerCPU, eventCPU);

    // Then: Do nothing
}

/*TEST(EventScheduler, GetWorkerIDMinimumJobs)
{
    // Given: MockQosManager, MockConfigManager, MockAffinityManager, EventScheduler
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockConfigManager> mockConfigManager;
    NiceMock<MockAffinityManager> mockAffinityManager;
    ON_CALL(mockConfigManager, GetValue).WillByDefault(
        [this] (string module, string key, void* value, ConfigType type)
        {
            *(bool*)value = true;
            return static_cast<int>(EID(SUCCESS));
        });
    ON_CALL(mockAffinityManager, GetTotalCore()).WillByDefault(Return(10));
    ON_CALL(mockAffinityManager, GetNumaIdFromCoreId).WillByDefault(
        [this] (uint32_t index)
        {
            return index % 2;
        });
    EventScheduler eventSchechduler {&mockQosManager, &mockConfigManager, &mockAffinityManager};
    cpu_set_t schedulerCPU, eventCPU;
    CPU_ZERO(&schedulerCPU);
    CPU_SET(8, &schedulerCPU);
    CPU_ZERO(&eventCPU);
    CPU_SET(7, &eventCPU);
    eventSchechduler.Initialize(1, schedulerCPU, eventCPU);
    uint32_t actual, expected = 0;

    // When: Call GetWorkerIDMinimumJobs
    actual = eventSchechduler.GetWorkerIDMinimumJobs(1);

    // Then: Expect to return 0
    EXPECT_EQ(actual, expected);
}
*/
TEST(EventScheduler, EnqueueEvent_VarietyInputs)
{
    // Given: EventScheduler, EventSmartPtr
    // Given: MockQosManager, MockConfigManager, MockAffinityManager, EventScheduler
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockConfigManager> mockConfigManager;
    NiceMock<MockAffinityManager> mockAffinityManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(static_cast<int>(EID(SUCCESS))));
    ON_CALL(mockAffinityManager, GetTotalCore()).WillByDefault(Return(10));
    ON_CALL(mockAffinityManager, GetNumaIdFromCoreId(_)).WillByDefault(Return(0));
    EventScheduler eventSchechduler{&mockQosManager, &mockConfigManager, &mockAffinityManager};
    cpu_set_t schedulerCPU, eventCPU;
    CPU_ZERO(&schedulerCPU);
    CPU_SET(1, &schedulerCPU);
    CPU_ZERO(&eventCPU);
    CPU_SET(2, &eventCPU);

    // When: Call Initialize
    eventSchechduler.Initialize(1, schedulerCPU, eventCPU);
    auto eventFront = std::make_shared<StubEventES>(true, BackendEvent_Unknown);
    auto eventFlush = std::make_shared<StubEventES>(false, BackendEvent_Flush);
    auto eventGC = std::make_shared<StubEventES>(false, BackendEvent_GC);
    auto eventUserRebuild = std::make_shared<StubEventES>(false, BackendEvent_UserdataRebuild);
    auto eventMetaRebuild = std::make_shared<StubEventES>(false, BackendEvent_MetadataRebuild);
    auto eventMetaIO = std::make_shared<StubEventES>(false, BackendEvent_MetaIO);
    auto eventFrontIO = std::make_shared<StubEventES>(false, BackendEvent_FrontendIO);

    // When: Call EnqueueEvent with variety inputs
    eventSchechduler.EnqueueEvent(eventFront);
    eventSchechduler.EnqueueEvent(eventFlush);
    eventSchechduler.EnqueueEvent(eventGC);
    eventSchechduler.EnqueueEvent(eventUserRebuild);
    eventSchechduler.EnqueueEvent(eventMetaRebuild);
    eventSchechduler.EnqueueEvent(eventMetaIO);
    eventSchechduler.EnqueueEvent(eventFrontIO);

    // Then: Do nothing
}

TEST(EventScheduler, DequeueEvent_)
{
    // Given:
    // When:
    // Then:
}

TEST(EventScheduler, Run_)
{
    // Given:
    // When:
    // Then:
}

} // namespace pos
