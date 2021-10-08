#include "src/event_scheduler/event_scheduler.h"

#include <assert.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

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
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(EventScheduler, EventScheduler_Stack)
{
    // Given: Set MockQosManager
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, GetEventWeightWRR(_)).WillByDefault(Return(-1));
    ON_CALL(mockQosManager, _Finalize()).WillByDefault(Return());

    // When: Create EventScheduler
    EventScheduler eventSechduler{&mockQosManager};

    // Then: Do nothing

    QosSpdkManager::unregistrationComplete = true; // Escape QosSpdkManager::Finalize infinite loop
}

TEST(EventScheduler, EventScheduler_Heap)
{
    // Given: Set MockQosManager
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, GetEventWeightWRR(_)).WillByDefault(Return(-1));
    ON_CALL(mockQosManager, _Finalize()).WillByDefault(Return());

    // When: Create EventScheduler
    EventScheduler* eventSechduler = new EventScheduler{&mockQosManager};
    delete eventSechduler;

    // Then: Do nothing
}

TEST(EventScheduler, Initialize_InvalidArgs)
{
    // Given: Set EventScheduler, cpu_set_t
    NiceMock<MockQosManager> mockQosManager;
    EventScheduler eventScheduler{&mockQosManager};
    ON_CALL(mockQosManager, GetEventWeightWRR(_)).WillByDefault(Return(-1));
    ON_CALL(mockQosManager, _Finalize()).WillByDefault(Return());
    uint32_t workerCountInput = 3;
    cpu_set_t schedulerCPUInput, eventCPUSetInput;
    CPU_ZERO(&schedulerCPUInput);
    CPU_SET(0, &schedulerCPUInput);
    CPU_ZERO(&eventCPUSetInput);
    CPU_SET(1, &eventCPUSetInput);

    // When: Call Initialize
    EXPECT_DEATH(eventScheduler.Initialize(workerCountInput, schedulerCPUInput, eventCPUSetInput), "");

    // Then: Do nothing
}

TEST(EventScheduler, Initialize_ValidArgs)
{
    // Given: Set EventScheduler, cpu_set_t
    NiceMock<MockQosManager> mockQosManager;
    EventScheduler eventScheduler{&mockQosManager};
    ON_CALL(mockQosManager, GetEventWeightWRR(_)).WillByDefault(Return(-1));
    ON_CALL(mockQosManager, _Finalize()).WillByDefault(Return());
    uint32_t workerCountInput = 1;
    cpu_set_t schedulerCPUInput, eventCPUSetInput;
    CPU_ZERO(&schedulerCPUInput);
    CPU_SET(0, &schedulerCPUInput);
    CPU_ZERO(&eventCPUSetInput);
    CPU_SET(1, &eventCPUSetInput);

    // When: Call Initialize
    eventScheduler.Initialize(workerCountInput, schedulerCPUInput, eventCPUSetInput);

    // Then: Do nothing
}

TEST(EventScheduler, GetWorkerIDMinimumJobs_Simple)
{
    // Given: Set EventScheduler, cpu_set_t, Initialize
    NiceMock<MockQosManager> mockQosManager;
    EventScheduler eventScheduler{&mockQosManager};
    ON_CALL(mockQosManager, GetEventWeightWRR(_)).WillByDefault(Return(-1));
    ON_CALL(mockQosManager, _Finalize()).WillByDefault(Return());
    uint32_t workerCountInput = 1;
    cpu_set_t schedulerCPUInput, eventCPUSetInput;
    CPU_ZERO(&schedulerCPUInput);
    CPU_SET(0, &schedulerCPUInput);
    CPU_ZERO(&eventCPUSetInput);
    CPU_SET(1, &eventCPUSetInput);
    eventScheduler.Initialize(workerCountInput, schedulerCPUInput, eventCPUSetInput);
    uint32_t actual, expected = 0;

    // When: Call GetWorkerIDMinimumJobs
    actual = eventScheduler.GetWorkerIDMinimumJobs(0);

    // Then: Expect return value is zero
    EXPECT_EQ(actual, expected);
}

TEST(EventScheduler, EnqueueEvent_FrontendIO)
{
    // Given: Set EventScheduler, MockEvent
    NiceMock<MockQosManager> mockQosManager;
    EventScheduler eventScheduler{&mockQosManager};
    ON_CALL(mockQosManager, GetEventWeightWRR(_)).WillByDefault(Return(-1));
    ON_CALL(mockQosManager, _Finalize()).WillByDefault(Return());
    auto mockEventSharedPtr = std::make_shared<MockEvent>(true);
    ON_CALL(*mockEventSharedPtr.get(), IsFrontEnd()).WillByDefault(Return(true));
    int eventTypeCount = 0;
    ON_CALL(*mockEventSharedPtr.get(), GetEventType()).WillByDefault([&eventTypeCount]()
    {
        eventTypeCount++;
        if (1 == eventTypeCount)
        {
            return BackendEvent_Unknown;
        }
        return BackendEvent_FrontendIO;
    });
    EXPECT_CALL(*mockEventSharedPtr.get(), GetEventType()).Times(4);
    EXPECT_CALL(*mockEventSharedPtr.get(), SetEventType(_)).Times(1);
    EXPECT_CALL(*mockEventSharedPtr.get(), IsFrontEnd()).Times(AtLeast(1));

    // When: Call EnqueueEvent
    eventScheduler.EnqueueEvent(mockEventSharedPtr);

    // Then: Check via DequeueEvents
    auto eventQ = eventScheduler.DequeueEvents();
    EXPECT_EQ(1, eventQ.size());
    EXPECT_TRUE(BackendEvent_FrontendIO == eventQ.front().get()->GetEventType());
}

TEST(EventScheduler, EnqueueEvent_NotFrontendTypes)
{
    // Given: Set EventScheduler, MockEvents
    NiceMock<MockQosManager> mockQosManager;
    EventScheduler eventScheduler{&mockQosManager};
    ON_CALL(mockQosManager, GetNoContentionCycles()).WillByDefault(Return(20));
    ON_CALL(mockQosManager, GetEventWeightWRR(_)).WillByDefault(Return(-1));
    ON_CALL(mockQosManager, IsFeQosEnabled()).WillByDefault(Return(false));
    ON_CALL(mockQosManager, _Finalize()).WillByDefault(Return());
    auto mockEventSharedPtr1 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr1.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr1.get(), GetEventType()).WillByDefault(Return(BackendEvent_Flush));
    EXPECT_CALL(*mockEventSharedPtr1.get(), GetEventType()).Times(AtLeast(3));
    EXPECT_CALL(*mockEventSharedPtr1.get(), IsFrontEnd()).Times(AtLeast(1));
    auto mockEventSharedPtr2 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr2.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr2.get(), GetEventType()).WillByDefault(Return(BackendEvent_GC));
    EXPECT_CALL(*mockEventSharedPtr2.get(), GetEventType()).Times(AtLeast(3));
    EXPECT_CALL(*mockEventSharedPtr2.get(), IsFrontEnd()).Times(AtLeast(1));
    auto mockEventSharedPtr3 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr3.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr3.get(), GetEventType()).WillByDefault(Return(BackendEvent_UserdataRebuild));
    EXPECT_CALL(*mockEventSharedPtr3.get(), GetEventType()).Times(AtLeast(3));
    EXPECT_CALL(*mockEventSharedPtr3.get(), IsFrontEnd()).Times(AtLeast(1));
    auto mockEventSharedPtr4 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr4.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr4.get(), GetEventType()).WillByDefault(Return(BackendEvent_MetadataRebuild));
    EXPECT_CALL(*mockEventSharedPtr4.get(), GetEventType()).Times(AtLeast(3));
    EXPECT_CALL(*mockEventSharedPtr4.get(), IsFrontEnd()).Times(AtLeast(1));
    auto mockEventSharedPtr5 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr5.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr5.get(), GetEventType()).WillByDefault(Return(BackendEvent_MetaIO));
    EXPECT_CALL(*mockEventSharedPtr5.get(), GetEventType()).Times(AtLeast(3));
    EXPECT_CALL(*mockEventSharedPtr5.get(), IsFrontEnd()).Times(AtLeast(1));
    auto mockEventSharedPtr6 = std::make_shared<MockEvent>();
    int eventTypeCount = 0;
    ON_CALL(*mockEventSharedPtr6.get(), GetEventType()).WillByDefault([&eventTypeCount]()
    {
        eventTypeCount++;
        if (1 == eventTypeCount)
        {
            return BackendEvent_Unknown;
        }
        return BackendEvent_FrontendIO;
    });
    ON_CALL(*mockEventSharedPtr6.get(), IsFrontEnd()).WillByDefault(Return(false));
    EXPECT_CALL(*mockEventSharedPtr6.get(), GetEventType()).Times(AtLeast(3));
    EXPECT_CALL(*mockEventSharedPtr6.get(), IsFrontEnd()).Times(AtLeast(1));
    auto mockEventSharedPtrErr = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtrErr.get(), GetEventType()).WillByDefault(Return(BackendEvent_End));
    EXPECT_CALL(*mockEventSharedPtrErr.get(), GetEventType()).Times(0);
    EXPECT_CALL(*mockEventSharedPtrErr.get(), IsFrontEnd()).Times(0);

    // When: Call EnqueueEvent
    eventScheduler.EnqueueEvent(mockEventSharedPtr1);
    eventScheduler.EnqueueEvent(mockEventSharedPtr2);
    eventScheduler.EnqueueEvent(mockEventSharedPtr3);
    eventScheduler.EnqueueEvent(mockEventSharedPtr4);
    eventScheduler.EnqueueEvent(mockEventSharedPtr5);
    eventScheduler.EnqueueEvent(mockEventSharedPtr6);
    EXPECT_DEATH(eventScheduler.EnqueueEvent(mockEventSharedPtrErr), "");

    // Then: Check via DequeueEvents
    auto eventQ = eventScheduler.DequeueEvents();
    EXPECT_EQ(6, eventQ.size());
}

TEST(EventScheduler, DequeueEvents_MultiEvents)
{
    // Given: Set EventScheduler, MockEvents, EnqueueEvent
    NiceMock<MockQosManager> mockQosManager;
    EventScheduler eventScheduler{&mockQosManager};
    ON_CALL(mockQosManager, GetEventWeightWRR(_)).WillByDefault(Return(-1));
    ON_CALL(mockQosManager, _Finalize()).WillByDefault(Return());
    auto mockEventSharedPtr1 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr1.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr1.get(), GetEventType()).WillByDefault(Return(BackendEvent_Flush));
    EXPECT_CALL(*mockEventSharedPtr1.get(), GetEventType()).Times(AtLeast(1));
    EXPECT_CALL(*mockEventSharedPtr1.get(), IsFrontEnd()).Times(AtLeast(1));
    auto mockEventSharedPtr2 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr2.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr2.get(), GetEventType()).WillByDefault(Return(BackendEvent_GC));
    EXPECT_CALL(*mockEventSharedPtr2.get(), GetEventType()).Times(AtLeast(1));
    EXPECT_CALL(*mockEventSharedPtr2.get(), IsFrontEnd()).Times(AtLeast(1));
    auto mockEventSharedPtr3 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr3.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr3.get(), GetEventType()).WillByDefault(Return(BackendEvent_FrontendIO));
    EXPECT_CALL(*mockEventSharedPtr3.get(), GetEventType()).Times(AtLeast(1));
    EXPECT_CALL(*mockEventSharedPtr3.get(), IsFrontEnd()).Times(AtLeast(1));
    eventScheduler.EnqueueEvent(mockEventSharedPtr1);
    eventScheduler.EnqueueEvent(mockEventSharedPtr2);
    eventScheduler.EnqueueEvent(mockEventSharedPtr3);

    // When: Call DequeueEvents
    auto eventQ = eventScheduler.DequeueEvents();

    // Then: Verify enqueued events
    int qSize = eventQ.size();
    EXPECT_EQ(3, qSize);
    for (int i = 0; i < qSize; i++)
    {
        auto event = eventQ.front().get();
        eventQ.pop();
        if (0 == i)
        {
            EXPECT_TRUE(BackendEvent_Flush == event->GetEventType());
        }
        else if (1 == i)
        {
            EXPECT_TRUE(BackendEvent_GC == event->GetEventType());
        }
        else if (2 == i)
        {
            EXPECT_TRUE(BackendEvent_FrontendIO == event->GetEventType());
        }
    }
    EXPECT_EQ(0, eventQ.size());
}

TEST(EventScheduler, Run_WithEvents)
{
    // Given: Set EventScheduler, Initialize, MockEvents, EnqueueEvent
    NiceMock<MockQosManager> mockQosManager;
    EventScheduler eventScheduler{&mockQosManager};
    ON_CALL(mockQosManager, GetEventWeightWRR(_)).WillByDefault(Return(-1));
    ON_CALL(mockQosManager, _Finalize()).WillByDefault(Return());
    uint32_t workerCountInput = 1;
    cpu_set_t schedulerCPUInput, eventCPUSetInput;
    CPU_ZERO(&schedulerCPUInput);
    CPU_SET(0, &schedulerCPUInput);
    CPU_ZERO(&eventCPUSetInput);
    CPU_SET(1, &eventCPUSetInput);
    auto mockEventSharedPtr1 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr1.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr1.get(), GetEventType()).WillByDefault(Return(BackendEvent_Flush));
    EXPECT_CALL(*mockEventSharedPtr1.get(), GetEventType()).Times(AtLeast(1));
    EXPECT_CALL(*mockEventSharedPtr1.get(), IsFrontEnd()).Times(AtLeast(1));
    eventScheduler.EnqueueEvent(mockEventSharedPtr1);
    auto mockEventSharedPtr2 = std::make_shared<MockEvent>();
    ON_CALL(*mockEventSharedPtr2.get(), IsFrontEnd()).WillByDefault(Return(false));
    ON_CALL(*mockEventSharedPtr2.get(), GetEventType()).WillByDefault(Return(BackendEvent_GC));
    EXPECT_CALL(*mockEventSharedPtr2.get(), GetEventType()).Times(AtLeast(1));
    EXPECT_CALL(*mockEventSharedPtr2.get(), IsFrontEnd()).Times(AtLeast(1));
    eventScheduler.EnqueueEvent(mockEventSharedPtr2);

    // When: Call Initialize to create thread and call Run
    eventScheduler.Initialize(workerCountInput, schedulerCPUInput, eventCPUSetInput);

    // Then: Do nothing
}

} // namespace pos
