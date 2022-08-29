#include "src/journal_manager/checkpoint/checkpoint_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/include/address_type.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/mapper/i_map_flush_mock.h"

using testing::_;
using testing::NiceMock;
using testing::Return;

const int ALLOCATOR_META_ID = 1000;
int numDirtyMaps = 5;
namespace pos
{
class CheckpointHandlerTestFixture : public ::testing::Test
{
public:
    CheckpointHandlerTestFixture(void)
    : mapFlush(nullptr),
      contextManager(nullptr),
      checkpointHandler(nullptr)
    {
    }

    virtual ~CheckpointHandlerTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        // Given
        mapFlush = new NiceMock<MockIMapFlush>;
        contextManager = new NiceMock<MockIContextManager>;
    }

    virtual void
    TearDown(void)
    {
        delete mapFlush;
        delete contextManager;

        delete checkpointHandler;
    }

    MapList
    GenerateDummyDirtyPageList(int numMaps)
    {
        // Generate dummy map list start from 0
        MapList dirty;

        for (int mapId = 0; mapId < numMaps; mapId++)
        {
            dirty.emplace(mapId);
        }

        return dirty;
    }

    void
    ExpectFlushDirtyMpages(MapList pendingDirtyMaps)
    {
        for (auto mapId : pendingDirtyMaps)
        {
            EXPECT_CALL(*mapFlush, FlushDirtyMpages(mapId, _)).WillOnce(Return(0));
        }
    }

protected:
    NiceMock<MockIMapFlush>* mapFlush;
    NiceMock<MockIContextManager>* contextManager;

    CheckpointHandler* checkpointHandler;
};

TEST_F(CheckpointHandlerTestFixture, Start_testIfCheckpointStartSuccessfullywithSingleDirtMapList)
{
    // Given
    checkpointHandler = new CheckpointHandler(0);
    checkpointHandler->Init(mapFlush, contextManager, nullptr);

    // When : Succeed flushing dirty map and allocator meta pages
    MapList pendingDirtyMaps = GenerateDummyDirtyPageList(1);
    ExpectFlushDirtyMpages(pendingDirtyMaps);
    EXPECT_CALL(*contextManager, FlushContexts).WillOnce(Return(0));

    // Then : Will restore the active stipre tail to this stripe
    EXPECT_EQ(checkpointHandler->Start(pendingDirtyMaps, nullptr), 0);
    EXPECT_EQ(checkpointHandler->GetStatus(), WAITING_FOR_FLUSH_DONE);
}

TEST_F(CheckpointHandlerTestFixture, Start_testIfCheckpointStartSuccessfullywithSeveralDirtMapList)
{
    // Given
    checkpointHandler = new CheckpointHandler(0);
    checkpointHandler->Init(mapFlush, contextManager, nullptr);

    // When : Succeed flushing dirty map and allocator meta pages
    MapList pendingDirtyMaps = GenerateDummyDirtyPageList(numDirtyMaps);
    ExpectFlushDirtyMpages(pendingDirtyMaps);
    EXPECT_CALL(*contextManager, FlushContexts).WillOnce(Return(0));

    // Then : Will restore the active stipre tail to this stripe
    EXPECT_EQ(checkpointHandler->Start(pendingDirtyMaps, nullptr), 0);
    EXPECT_EQ(checkpointHandler->GetStatus(), WAITING_FOR_FLUSH_DONE);
}

// TODO (cheolho.kang) Add later
TEST_F(CheckpointHandlerTestFixture, Start_testIfFlushCompletedImmediatelyDuringStartCheckpoint)
{
}

TEST_F(CheckpointHandlerTestFixture, Start_testIfCheckpointFailedWhenFlushDirtyMpagesFailed)
{
    // Given
    checkpointHandler = new CheckpointHandler(0);
    checkpointHandler->Init(mapFlush, contextManager, nullptr);

    // When : Failed to flushing dirty map pages
    MapList pendingDirtyMaps = GenerateDummyDirtyPageList(numDirtyMaps);
    EXPECT_CALL(*mapFlush, FlushDirtyMpages).WillOnce(Return(-1));

    // Then : Checkpoint should be started
    EXPECT_TRUE(checkpointHandler->Start(pendingDirtyMaps, nullptr) != 0);
    EXPECT_EQ(checkpointHandler->GetStatus(), STARTED);
}

TEST_F(CheckpointHandlerTestFixture, Start_testIfCheckpointFailedWhencontextManagersFlushFailed)
{
    // Given
    checkpointHandler = new CheckpointHandler(0);
    checkpointHandler->Init(mapFlush, contextManager, nullptr);

    // When : Succeed to flushing dirty map pages and failed flushing allocator meta pages
    MapList pendingDirtyMaps = GenerateDummyDirtyPageList(numDirtyMaps);
    ExpectFlushDirtyMpages(pendingDirtyMaps);
    EXPECT_CALL(*contextManager, FlushContexts).WillOnce(Return(-1));

    // Then : Checkpoint should be started
    EXPECT_TRUE(checkpointHandler->Start(pendingDirtyMaps, nullptr) != 0);
    EXPECT_EQ(checkpointHandler->GetStatus(), WAITING_FOR_FLUSH_DONE);
}

TEST_F(CheckpointHandlerTestFixture, FlushCompleted_testIfCheckpointCompleted)
{
    // Given
    int numMapsToFlush = 1;
    int numMapsFlushed = 0;

    MockEventScheduler eventScheduler;
    ON_CALL(eventScheduler, EnqueueEvent).WillByDefault([&](EventSmartPtr event)
    {
        event->Execute();
    });

    EventSmartPtr checkpointCompletion(new MockEvent());
    checkpointHandler = new CheckpointHandler(numMapsToFlush, numMapsFlushed, checkpointCompletion, 0);
    checkpointHandler->Init(nullptr, contextManager, &eventScheduler);

    // Then: Callback event should be executed
    EXPECT_CALL(*(MockEvent*)(checkpointCompletion.get()), Execute);

    // When: All dirty maps and allocator meta are flushed
    EXPECT_TRUE(checkpointHandler->FlushCompleted(0, 0) == 0);
    EXPECT_TRUE(checkpointHandler->FlushCompleted(ALLOCATOR_META_ID, 0) == 0);

    // Then: Checkpoint status should be changed to COMPLETED
    EXPECT_TRUE(checkpointHandler->GetStatus() == COMPLETED);
}

TEST_F(CheckpointHandlerTestFixture, FlushCompleted_testIfCheckpointFailedWhenMapFlushUncompleted)
{
    // Given: Map is not flushed yet
    int numMapsToFlush = 2;
    int numMapsFlushed = 0;

    MockEventScheduler eventScheduler;
    ON_CALL(eventScheduler, EnqueueEvent).WillByDefault([&](EventSmartPtr event)
    {
        event->Execute();
    });

    checkpointHandler = new CheckpointHandler(numMapsToFlush, numMapsFlushed, nullptr, 0);
    checkpointHandler->Init(nullptr, contextManager, &eventScheduler);

    // Then: Checkpoint status should not be COMPLETED
    EXPECT_TRUE(checkpointHandler->GetStatus() != COMPLETED);

    // When: Allocatator meta flush is completed
    EXPECT_TRUE(checkpointHandler->FlushCompleted(ALLOCATOR_META_ID, 0) == 0);

    // Then: Checkpoint status should not be changed to COMPLETED
    EXPECT_TRUE(checkpointHandler->GetStatus() != COMPLETED);
}
} // namespace pos
