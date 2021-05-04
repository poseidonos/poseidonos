#include "src/journal_manager/checkpoint/checkpoint_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/include/address_type.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_observer_mock.h"
#include "test/unit-tests/mapper/i_map_flush_mock.h"

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
    : checkpointObserver(nullptr),
      mapFlush(nullptr),
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
        checkpointObserver = new NiceMock<MockCheckpointObserver>;
        mapFlush = new NiceMock<MockIMapFlush>;
        contextManager = new NiceMock<MockIContextManager>;
    }

    virtual void
    TearDown(void)
    {
        delete checkpointObserver;
        delete mapFlush;
        delete contextManager;

        delete checkpointHandler;
    }

    MapPageList
    GenerateDummyDirtyPageList(int numMaps)
    {
        // Generate dummy mpage list
        MpageList dirty;
        MapPageList dirtyPages;

        dirty.insert(0);
        for (int mapId = 0; mapId < numMaps; mapId++)
        {
            dirtyPages[mapId].insert(dirty.begin(), dirty.end());
        }

        return dirtyPages;
    }

protected:
    NiceMock<MockCheckpointObserver>* checkpointObserver;
    NiceMock<MockIMapFlush>* mapFlush;
    NiceMock<MockIContextManager>* contextManager;

    CheckpointHandler* checkpointHandler;
};

TEST_F(CheckpointHandlerTestFixture, Start_testIfCheckpointStartSuccessfullywithSingleDirtMapList)
{
    // Given
    checkpointHandler = new CheckpointHandler(checkpointObserver);
    checkpointHandler->Init(mapFlush, contextManager);

    // When : Succeed flushing dirty map and allocator meta pages
    MapPageList pendingDirtyPages = GenerateDummyDirtyPageList(1);
    EXPECT_CALL(*mapFlush, FlushDirtyMpages).WillOnce(Return(0));
    EXPECT_CALL(*contextManager, FlushContextsAsync).WillOnce(Return(0));

    // Then : Will restore the active stipre tail to this stripe
    EXPECT_EQ(checkpointHandler->Start(pendingDirtyPages), 0);
    EXPECT_EQ(checkpointHandler->GetStatus(), WAITING_FOR_FLUSH_DONE);
}

TEST_F(CheckpointHandlerTestFixture, Start_testIfCheckpointStartSuccessfullywithSeveralDirtMapList)
{
    // Given
    checkpointHandler = new CheckpointHandler(checkpointObserver);
    checkpointHandler->Init(mapFlush, contextManager);

    // When : Succeed flushing dirty map and allocator meta pages
    MapPageList pendingDirtyPages = GenerateDummyDirtyPageList(numDirtyMaps);
    EXPECT_CALL(*mapFlush, FlushDirtyMpages)
        .Times(numDirtyMaps)
        .WillRepeatedly(Return(0));
    EXPECT_CALL(*contextManager, FlushContextsAsync).WillOnce(Return(0));

    // Then : Will restore the active stipre tail to this stripe
    EXPECT_EQ(checkpointHandler->Start(pendingDirtyPages), 0);
    EXPECT_EQ(checkpointHandler->GetStatus(), WAITING_FOR_FLUSH_DONE);
}

// TODO (cheolho.kang) Add later
TEST_F(CheckpointHandlerTestFixture, Start_testIfFlushCompletedImmediatelyDuringStartCheckpoint)
{
}

TEST_F(CheckpointHandlerTestFixture, Start_testIfCheckpointFailedWhenFlushDirtyMpagesFailed)
{
    // Given
    checkpointHandler = new CheckpointHandler(checkpointObserver);
    checkpointHandler->Init(mapFlush, contextManager);

    // When : Failed to flushing dirty map pages
    MapPageList pendingDirtyPages = GenerateDummyDirtyPageList(numDirtyMaps);
    EXPECT_CALL(*mapFlush, FlushDirtyMpages).WillOnce(Return(-1));

    // Then : Checkpoint should be started
    EXPECT_TRUE(checkpointHandler->Start(pendingDirtyPages) != 0);
    EXPECT_EQ(checkpointHandler->GetStatus(), STARTED);
}

TEST_F(CheckpointHandlerTestFixture, Start_testIfCheckpointFailedWhencontextManagersFlushFailed)
{
    // Given
    checkpointHandler = new CheckpointHandler(checkpointObserver);
    checkpointHandler->Init(mapFlush, contextManager);

    // When : Succeed to flushing dirty map pages and failed flushing allocator meta pages
    MapPageList pendingDirtyPages = GenerateDummyDirtyPageList(numDirtyMaps);
    EXPECT_CALL(*mapFlush, FlushDirtyMpages)
        .Times(numDirtyMaps)
        .WillRepeatedly(Return(0));
    EXPECT_CALL(*contextManager, FlushContextsAsync).WillOnce(Return(-1));

    // Then : Checkpoint should be started
    EXPECT_TRUE(checkpointHandler->Start(pendingDirtyPages) != 0);
    EXPECT_EQ(checkpointHandler->GetStatus(), WAITING_FOR_FLUSH_DONE);
}

TEST_F(CheckpointHandlerTestFixture, FlushCompleted_testIfCheckpointCompletely)
{
    // Given
    checkpointHandler = new CheckpointHandler(checkpointObserver);

    // When : All dirty mapss are flushed
    int numMapsToFlush = 1;
    int numMapsFlushed = 0;
    checkpointHandler = new CheckpointHandler(checkpointObserver, numMapsToFlush, numMapsFlushed);
    EXPECT_TRUE(checkpointHandler->FlushCompleted(0) == 0);

    // Then : Checkpoint status is not completed yet
    EXPECT_TRUE(checkpointHandler->GetStatus() != COMPLETED);

    // When : Allocatator meta and map flsuh are completed
    EXPECT_TRUE(checkpointHandler->FlushCompleted(ALLOCATOR_META_ID) == 0);

    // Then : Checkpoint status should be completed
    EXPECT_TRUE(checkpointHandler->GetStatus() == COMPLETED);
}

TEST_F(CheckpointHandlerTestFixture, FlushCompleted_testIfCheckpointFailedWhenMapFlushUncompleted)
{
    // When : Flushing dirty map pages is not completed fully yet
    int numMapsToFlush = 2;
    int numMapsFlushed = 0;
    checkpointHandler = new CheckpointHandler(checkpointObserver, numMapsToFlush, numMapsFlushed);
    EXPECT_TRUE(checkpointHandler->FlushCompleted(0) == 0);

    // Then : Checkpoint status is not completed
    EXPECT_TRUE(checkpointHandler->GetStatus() != COMPLETED);

    // When : Allocatator meta flsuh are completed when dirty map page is not yet flushed
    EXPECT_TRUE(checkpointHandler->FlushCompleted(ALLOCATOR_META_ID) == 0);

    // Then : Checkpoint status is not completed
    EXPECT_TRUE(checkpointHandler->GetStatus() != COMPLETED);
}
} // namespace pos
