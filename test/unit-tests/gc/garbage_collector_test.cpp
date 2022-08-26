#include "src/gc/garbage_collector.h"

#include <gtest/gtest.h>
#include <src/state/include/situation_type.h>
#include <test/unit-tests/gc/copier_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/event_scheduler/event_scheduler_mock.h>
#include <test/unit-tests/state/interface/i_state_control_mock.h>
#include <test/unit-tests/state/state_context_mock.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Test;
namespace pos
{
class GarbageCollectorTestFixture : public ::testing::Test
{
public:
    GarbageCollectorTestFixture(void)
    : gc(nullptr),
      array(nullptr),
      copier(nullptr),
      eventScheduler(nullptr),
      stateControl(nullptr)
    {
    }

    virtual ~GarbageCollectorTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        array = new NiceMock<MockIArrayInfo>;
        copier =  new NiceMock<MockCopier>(0, 0, nullptr, nullptr,
                &partitionLogicalSize, nullptr, nullptr, nullptr, nullptr, nullptr);
        eventScheduler = new NiceMock<MockEventScheduler>;
        stateControl = new NiceMock<MockIStateControl>;

        copierPtr = shared_ptr<Copier>(copier);

        copierFactory = [](GcStatus* gcStatus, IArrayInfo* array, CopierSmartPtr inputEvent)
        {
            return inputEvent;
        };

        gc = new GarbageCollector(array, stateControl, copierPtr, copierFactory, eventScheduler);
    }

    virtual void
    TearDown(void)
    {
        delete gc;
        delete array;
        delete eventScheduler;
        delete stateControl;
        copierPtr = nullptr;
    }

protected:
    GarbageCollector* gc;
    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockCopier>* copier;
    NiceMock<MockEventScheduler>* eventScheduler;
    NiceMock<MockIStateControl>* stateControl;
    CopierSmartPtr copierPtr;
    function<CopierSmartPtr(GcStatus*, IArrayInfo*, CopierSmartPtr)> copierFactory;

    const PartitionLogicalSize* udSize = &partitionLogicalSize;

    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0/* not interesting */,
    .blksPerChunk = 4,
    .blksPerStripe = 16,
    .chunksPerStripe = 4,
    .stripesPerSegment = 32,
    .totalStripes = 3200,
    .totalSegments = 100,
    };
};

TEST_F(GarbageCollectorTestFixture, GarbageCollector_testIfMakeCopierPtrFailWhenGarbageCollectorInit)
{
    // given nullptr injection for copier smart ptr
    GarbageCollector* testGc = new GarbageCollector(array, stateControl, nullptr, copierFactory, eventScheduler);

    // when garbage collector init
    // then return fail with "gc cannot create copier" event id
    EXPECT_TRUE(testGc->Init() == static_cast<int>(EID(GC_CANNOT_CREATE_COPIER)));
    delete testGc;
}

TEST_F(GarbageCollectorTestFixture, GarbageCollector_testIfStateSubscribeAndEnqueueCopierEventWhenGarbageCollectorInit)
{
    // given state control, event scheduler when create garbage collector
    EXPECT_CALL(*stateControl, Subscribe).Times(1);
    EXPECT_CALL(*eventScheduler, EnqueueEvent(_)).Times(1);

    // when garbage collector init 
    // then subscribe state manager, enqueue copier event and return success
    EXPECT_TRUE(gc->Init() == 0);
}

TEST_F(GarbageCollectorTestFixture, Start_testGcStartSuccessful)
{
    // given event scheduler
    EXPECT_CALL(*eventScheduler, EnqueueEvent).Times(1);

    // when garbage collector start
    // then enqueue copier event and return success
    EXPECT_TRUE(gc->Start() == 0);
}

TEST_F(GarbageCollectorTestFixture, End_testGcEndAfterGcStarted)
{
    // given start gc
    EXPECT_CALL(*eventScheduler, EnqueueEvent).Times(1);
    EXPECT_TRUE(gc->Start() == 0);
    EXPECT_CALL(*copier, Stop).Times(1);
    EXPECT_CALL((*copier), IsStopped).WillOnce(Return(false));
    EXPECT_CALL((*copier), ReadyToEnd).Times(1);

    // when end gc
    gc->End();
    // then isRunning is false
    EXPECT_TRUE(gc->GetGcRunning() == false);
}

TEST_F(GarbageCollectorTestFixture, End_testIfGcEndDoNothingWhenGcisnotRunning)
{
    // given just create garbage collector
    // when end gc
    // then do nothing
    gc->End(); // trivial exception handling
}

TEST_F(GarbageCollectorTestFixture, StateChanged_Invoked)
{
    gc->StateChanged(nullptr, nullptr); // trivial state changed
}

TEST_F(GarbageCollectorTestFixture, Dispose_testIfDisposeAfterGcStart)
{
    // given start gc
    EXPECT_CALL(*eventScheduler, EnqueueEvent).Times(1);
    EXPECT_TRUE(gc->Start() == 0);

    // when gc dispose
    // then unsubscribe, copier stopped and readyToEnd
    EXPECT_CALL(*stateControl, Unsubscribe).Times(1);
    EXPECT_CALL(*copier, Stop).Times(1);
    EXPECT_CALL((*copier), IsStopped).WillOnce(Return(false));
    EXPECT_CALL((*copier), ReadyToEnd).Times(1);
    gc->Dispose();
    EXPECT_TRUE(gc->GetGcRunning() == false);
}

TEST_F(GarbageCollectorTestFixture, Pause_testIfGcPauseWhenCopierExists)
{
    // given create garbage collector and copier
    EXPECT_CALL(*eventScheduler, EnqueueEvent).Times(1);
    EXPECT_TRUE(gc->Start() == 0);
    EXPECT_CALL((*copier), Pause).Times(1);
    // when gc puase
    // then call copier pause
    gc->Pause(); // trival copier pause
}

TEST_F(GarbageCollectorTestFixture, Resume_testIfGcResumeWhenCopierExists)
{
    EXPECT_CALL(*eventScheduler, EnqueueEvent).Times(1);
    EXPECT_TRUE(gc->Start() == 0);
    EXPECT_CALL((*copier), Resume).Times(1);
    gc->Resume(); // trival copier resume
}

TEST_F(GarbageCollectorTestFixture, IsPaused_testIfGcIsPausedWhenCopierExists)
{
    EXPECT_CALL(*eventScheduler, EnqueueEvent).Times(1);
    EXPECT_TRUE(gc->Start() == 0);
    EXPECT_CALL((*copier), IsPaused).Times(1);
    gc->IsPaused(); // trival copier IsPaused
}

TEST_F(GarbageCollectorTestFixture, DisableThresholdCheck_testIfDisableThresholdCheckBasedOnCopierReturn)
{
    // given create copier
    EXPECT_CALL(*eventScheduler, EnqueueEvent).Times(1);
    EXPECT_TRUE(gc->Start() == 0);

    // when copier return false
    EXPECT_CALL((*copier), IsEnableThresholdCheck).WillOnce(Return(false));
    // then gc DisableThresholdCheck return -1
    EXPECT_TRUE(gc->DisableThresholdCheck() == -1);

    // when copier return true
    EXPECT_CALL((*copier), IsEnableThresholdCheck).WillOnce(Return(true));

    // then called copier disableThresholdCheck, gc disableThresholdCheck return 0
    EXPECT_CALL((*copier), DisableThresholdCheck).Times(1);
    EXPECT_TRUE(gc->DisableThresholdCheck() == 0);
}

TEST_F(GarbageCollectorTestFixture, IsEnabled_testIfIsEnabledBasedOnState)
{
    // given mock state context
    MockStateContext mockStateContext("sender", SituationEnum::DEFAULT);

    // when mock state context return normal state
    EXPECT_CALL((*stateControl), GetState).WillRepeatedly(Return(&mockStateContext));
    EXPECT_CALL((mockStateContext), ToStateType).WillOnce(Return(StateEnum::NORMAL));
    // then is enabled return enable code
    EXPECT_TRUE(gc->IsEnabled() == 0);

    // when mock state context return stop state
    EXPECT_CALL((mockStateContext), ToStateType).WillOnce(Return(StateEnum::STOP));
    // then is enabled return disable code
    EXPECT_TRUE(gc->IsEnabled() != 0);

    // given garbage start
    EXPECT_CALL(*eventScheduler, EnqueueEvent).Times(1);
    EXPECT_TRUE(gc->Start() == 0);

    // when mock state context return normal state
    EXPECT_CALL((mockStateContext), ToStateType).WillOnce(Return(StateEnum::NORMAL));
    // thrn is enabled return enable code
    EXPECT_TRUE(gc->IsEnabled() == 0);
}

TEST_F(GarbageCollectorTestFixture, GetGcRunning_testIfGetGcStatusData)
{
    gc->GetStartTime(); // trival get gc status start time
    gc->GetEndTime(); // trival get gc status end time
    EXPECT_TRUE(gc->GetGcRunning() == false); // trival get gc running state
}

TEST_F(GarbageCollectorTestFixture, Shutdown_testIfShutdownAfterGcStart)
{
    // given gc start
    EXPECT_CALL(*eventScheduler, EnqueueEvent).Times(1);
    EXPECT_TRUE(gc->Start() == 0);

    // when gc shutdown
    // then unsubscribe, copier stopped and readyToEnd
    EXPECT_CALL(*stateControl, Unsubscribe).Times(1);
    EXPECT_CALL(*copier, Stop).Times(1);
    EXPECT_CALL((*copier), IsStopped).WillOnce(Return(false));
    EXPECT_CALL((*copier), ReadyToEnd).Times(1);
    gc->Shutdown();
    EXPECT_TRUE(gc->GetGcRunning() == false);
}

TEST_F(GarbageCollectorTestFixture, Flush_Invoked)
{
    gc->Flush(); // trival no op for IMountSequence
}

} // namespace pos
