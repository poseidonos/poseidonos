#include "src/journal_manager/journal_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/dirty_map_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/log_group_releaser_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_factory_mock.h"
#include "test/unit-tests/journal_manager/log_write/buffer_offset_allocator_mock.h"
#include "test/unit-tests/journal_manager/log_write/journal_volume_event_handler_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_handler_mock.h"
#include "test/unit-tests/journal_manager/status/journal_status_provider_mock.h"
#include "test/unit-tests/journal_service/journal_service_mock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgReferee;

namespace pos
{
class JournalManagerTestFixture : public ::testing::Test
{
public:
    JournalManagerTestFixture(void)
    : config(nullptr),
      statusProvider(nullptr),
      logWriteHandler(nullptr),
      logWriteContextFactory(nullptr),
      volumeEventHandler(nullptr),
      logBuffer(nullptr),
      bufferAllocator(nullptr),
      logGroupReleaser(nullptr),
      dirtyMapManager(nullptr),
      logFilledNotifier(nullptr),
      replayHandler(nullptr),
      arrayInfo(nullptr),
      service(nullptr)
    {
    }
    virtual ~JournalManagerTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        config = new NiceMock<MockJournalConfiguration>;
        statusProvider = new NiceMock<MockJournalStatusProvider>;
        logWriteHandler = new NiceMock<MockLogWriteHandler>;
        logWriteContextFactory = new NiceMock<MockLogWriteContextFactory>;
        volumeEventHandler = new NiceMock<MockJournalVolumeEventHandler>;
        logBuffer = new NiceMock<MockJournalLogBuffer>;
        bufferAllocator = new NiceMock<MockBufferOffsetAllocator>;
        logGroupReleaser = new NiceMock<MockLogGroupReleaser>;
        checkpointManager = new NiceMock<MockCheckpointManager>;
        dirtyMapManager = new NiceMock<MockDirtyMapManager>;
        logFilledNotifier = new NiceMock<MockLogBufferWriteDoneNotifier>;
        callbackSequenceController = new NiceMock<MockCallbackSequenceController>;
        replayHandler = new NiceMock<MockReplayHandler>;
        arrayInfo = new NiceMock<MockIArrayInfo>;
        service = new NiceMock<MockJournalService>;

        journal = new JournalManager(config, statusProvider,
            logWriteContextFactory, logWriteHandler, volumeEventHandler, logBuffer,
            bufferAllocator, logGroupReleaser, checkpointManager, dirtyMapManager, logFilledNotifier,
            callbackSequenceController, replayHandler, arrayInfo, service);
    }

    virtual void
    TearDown(void)
    {
        delete journal;
        delete arrayInfo;
        delete service;
    }

protected:
    JournalManager* journal;

    // Journal Depend-On-Components
    NiceMock<MockJournalConfiguration>* config;
    NiceMock<MockJournalStatusProvider>* statusProvider;
    NiceMock<MockLogWriteHandler>* logWriteHandler;
    NiceMock<MockLogWriteContextFactory>* logWriteContextFactory;
    NiceMock<MockJournalVolumeEventHandler>* volumeEventHandler;
    NiceMock<MockJournalLogBuffer>* logBuffer;
    NiceMock<MockBufferOffsetAllocator>* bufferAllocator;
    NiceMock<MockLogGroupReleaser>* logGroupReleaser;
    NiceMock<MockCheckpointManager>* checkpointManager;
    NiceMock<MockDirtyMapManager>* dirtyMapManager;
    NiceMock<MockLogBufferWriteDoneNotifier>* logFilledNotifier;
    NiceMock<MockCallbackSequenceController>* callbackSequenceController;
    NiceMock<MockReplayHandler>* replayHandler;
    NiceMock<MockIArrayInfo>* arrayInfo;
    NiceMock<MockJournalService>* service;
};

TEST_F(JournalManagerTestFixture, Init_testWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    EXPECT_CALL(*config, IsEnabled()).WillRepeatedly(Return(false));

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal should be initialized with INVALID state
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNAL_INVALID);
}

TEST_F(JournalManagerTestFixture, Init_testWithJournalEnabledAndLogBufferNotExist)
{
    // Given: Journal config manager is configured to be enabled
    ON_CALL(*config, IsEnabled()).WillByDefault(Return(true));
    ON_CALL(*logBuffer, DoesLogFileExist).WillByDefault(Return(false));
    ON_CALL(*logBuffer, Create).WillByDefault(Return(0));

    // Then: All sub-modules should be initiailized
    EXPECT_CALL(*config, Init);
    EXPECT_CALL(*bufferAllocator, Init);
    EXPECT_CALL(*dirtyMapManager, Init);
    EXPECT_CALL(*logWriteContextFactory, Init);
    EXPECT_CALL(*logGroupReleaser, Init);
    EXPECT_CALL(*logWriteHandler, Init);
    EXPECT_CALL(*volumeEventHandler, Init);
    EXPECT_CALL(*replayHandler, Init);
    EXPECT_CALL(*statusProvider, Init);
    EXPECT_CALL(*logBuffer, Init);

    // Then: Log buffer should be created
    EXPECT_CALL(*logBuffer, Create);

    // Then: Log filled subscriber to be added in this sequence
    {
        InSequence s;
        EXPECT_CALL(*logFilledNotifier, Register(dirtyMapManager));
        EXPECT_CALL(*logFilledNotifier, Register(bufferAllocator));
        EXPECT_CALL(*logFilledNotifier, Register(logWriteHandler));
    }

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal manager should be ready
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNALING);
}

TEST_F(JournalManagerTestFixture, Init_testWithJournalEnabledAndLogBufferExist)
{
    // Given: Journal config manager is configured to be enabled
    ON_CALL(*config, IsEnabled()).WillByDefault(Return(true));
    ON_CALL(*logBuffer, DoesLogFileExist).WillByDefault(Return(true));

    // Then: JournalConfiguration should be initialized with the size of loaded log buffer
    uint64_t expectedLogBufferSize = 16 * 1024;
    uint64_t loadedLogBufferSize = 0;
    EXPECT_CALL(*logBuffer, Open(_)).WillOnce(DoAll(SetArgReferee<0>(expectedLogBufferSize), Return(0)));
    EXPECT_CALL(*config, Init(expectedLogBufferSize, _));

    // Then: All sub-modules should be initiailized
    EXPECT_CALL(*bufferAllocator, Init);
    EXPECT_CALL(*dirtyMapManager, Init);
    EXPECT_CALL(*logWriteContextFactory, Init);
    EXPECT_CALL(*logGroupReleaser, Init);
    EXPECT_CALL(*logWriteHandler, Init);
    EXPECT_CALL(*volumeEventHandler, Init);
    EXPECT_CALL(*replayHandler, Init);
    EXPECT_CALL(*statusProvider, Init);
    EXPECT_CALL(*logBuffer, Init);

    // Then: Log filled subscriber to be added in this sequence
    {
        InSequence s;
        EXPECT_CALL(*logFilledNotifier, Register(dirtyMapManager));
        EXPECT_CALL(*logFilledNotifier, Register(bufferAllocator));
        EXPECT_CALL(*logFilledNotifier, Register(logWriteHandler));
    }

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal manager should be ready
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNALING);
}

TEST_F(JournalManagerTestFixture, Dispose_testWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(false));

    // Then: Journal should be un-registered
    EXPECT_CALL(*service, Unregister);

    // When: Journal is disposed
    journal->Dispose();
}

TEST_F(JournalManagerTestFixture, Dispose_testWithJournalEnabled)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // Then: Journal should be un-registered and log buffer should be disposed
    EXPECT_CALL(*service, Unregister);
    EXPECT_CALL(*logBuffer, Dispose);

    // When: Journal is disposed
    journal->Dispose();
}

TEST_F(JournalManagerTestFixture, Shutdown_testWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(false));

    // Then: Journal should be un-registered and
    // log buffer should not be disposed, nor reset to log buffer
    EXPECT_CALL(*service, Unregister);
    EXPECT_CALL(*logBuffer, SyncResetAll).Times(0);
    EXPECT_CALL(*logBuffer, Dispose).Times(0);

    // When: Journal shutdowns
    journal->Shutdown();
}

TEST_F(JournalManagerTestFixture, Shutdown_testWithJournalEnabled)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // Then: Journal should be un-registered and log buffer should be disposed after reset
    EXPECT_CALL(*service, Unregister);
    {
        InSequence s;

        EXPECT_CALL(*logBuffer, SyncResetAll).Times(0);
        EXPECT_CALL(*logBuffer, Dispose);
    }

    // When: Journal shutdowns
    journal->Shutdown();
}

TEST_F(JournalManagerTestFixture, Init_testInitWhenLogBufferNotExist)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // When: Log buffer is not loaded
    EXPECT_CALL(*logBuffer, DoesLogFileExist).WillOnce(Return(false));

    // Then: Expect log buffer to be reset and journal to be registered to the service
    EXPECT_CALL(*logBuffer, Create).WillOnce(Return(0));
    EXPECT_CALL(*logBuffer, SyncResetAll).WillOnce(Return(0));
    EXPECT_CALL(*service, Register);

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal manager should be ready
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNALING);
}

TEST_F(JournalManagerTestFixture, Init_testInitWhenLogBufferLoaded)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // When: Log buffer is loaded
    EXPECT_CALL(*logBuffer, DoesLogFileExist).WillOnce(Return(true));

    // Then: Expect to start replay, and journal to be registered to the service
    EXPECT_CALL(*logBuffer, Open).WillOnce(Return(0));
    EXPECT_CALL(*replayHandler, Start).WillOnce(Return(0));
    EXPECT_CALL(*service, Register);

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal manager should be ready
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNALING);
}

TEST_F(JournalManagerTestFixture, IsEnabled_testWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(false));

    // When: Journal is asked if it's enabled
    // Then: Journal should be disabled
    EXPECT_TRUE(journal->IsEnabled() == false);
}

TEST_F(JournalManagerTestFixture, IsEnabled_testWithJournalEnabled)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // When: Journal is asked if it's enabled
    // Then: Journal should be enabled
    EXPECT_TRUE(journal->IsEnabled() == true);
}

TEST_F(JournalManagerTestFixture, AddBlockMapUpdatedLog_testIfFailsWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    ON_CALL(*config, IsEnabled).WillByDefault(Return(false));

    // When: Journal is requested to write block write done log
    // Then: Log write should be failed
    MpageList dummyList;
    EXPECT_TRUE(journal->AddBlockMapUpdatedLog(nullptr, dummyList, nullptr) < 0);
}

TEST_F(JournalManagerTestFixture, AddBlockMapUpdatedLog_testIfSuccessWithJournalEnabled)
{
    // Given: Journal config manager is configured to be enabled
    ON_CALL(*config, IsEnabled).WillByDefault(Return(true));

    // When: Journal is initialized beforehead
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal should request writing logs to write handler
    EXPECT_CALL(*logWriteHandler, AddLog);

    // When: Journal is requested to write block write done log
    // Then: Log write should be successfully requested
    MpageList dummyList;
    EXPECT_TRUE(journal->AddBlockMapUpdatedLog(nullptr, dummyList, nullptr) == 0);
}

TEST_F(JournalManagerTestFixture, AddStripeMapUpdatedLog_testIfFailsWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    ON_CALL(*config, IsEnabled).WillByDefault(Return(false));

    // When: Journal is requested to write stripe map updated log
    // Then: Log write should be failed
    MpageList dummyList;
    StripeAddr unmap = {.stripeId = UNMAP_STRIPE};
    EXPECT_TRUE(journal->AddStripeMapUpdatedLog(nullptr, unmap, dummyList, nullptr) < 0);
}

TEST_F(JournalManagerTestFixture, AddStripeMapUpdatedLog_testIfSuccessWithJournalEnabled)
{
    // Given: Journal config manager is configured to be disabled
    ON_CALL(*config, IsEnabled).WillByDefault(Return(true));

    // When: Journal is initialized beforehead
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal should request writing logs to write handler
    EXPECT_CALL(*logWriteHandler, AddLog);

    // When: Journal is requested to write stripe map updated log
    // Then: Log write should be successfully requested
    MpageList dummyList;
    StripeAddr unmap = {.stripeId = UNMAP_STRIPE};
    EXPECT_TRUE(journal->AddStripeMapUpdatedLog(nullptr, unmap, dummyList, nullptr) == 0);
}

TEST_F(JournalManagerTestFixture, AddGcStripeFlushedLog_testIfFailsWithJournalDisabled)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(false));

    // When: Journal is requested to write gc stripe flushed log
    // Then: Log write should be failed
    GcStripeMapUpdateList dummyMapUpdates;
    MapPageList dummyList;
    EXPECT_TRUE(journal->AddGcStripeFlushedLog(dummyMapUpdates, dummyList, nullptr) < 0);
}

TEST_F(JournalManagerTestFixture, AddGcStripeFlushedLog_testIfSuccessWithJournalEnabled)
{
    // Given: Journal config manager is configured to be enabled
    ON_CALL(*config, IsEnabled).WillByDefault(Return(true));

    // When: Journal is initialized beforehead
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal should request writing logs to write handler
    EXPECT_CALL(*logWriteHandler, AddLog);

    // When: Journal is requested to write gc stripe flushed log
    // Then: Log write should be successfully requested
    GcStripeMapUpdateList dummyMapUpdates;
    MapPageList dummyList;
    EXPECT_TRUE(journal->AddGcStripeFlushedLog(dummyMapUpdates, dummyList, nullptr) == 0);
}

} // namespace pos
