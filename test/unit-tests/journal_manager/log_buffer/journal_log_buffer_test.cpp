#include "src/journal_manager/log_buffer/journal_log_buffer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_buffer_io_context_factory_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_buffer_io_context_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_factory_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/map_update_log_write_context_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using testing::_;
using testing::NiceMock;
using testing::Return;

namespace pos
{
TEST(JournalLogBuffer, InitDataBuffer_testIfDataBufferInitialized)
{
    // Given : journalConfig is initialized
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);
    NiceMock<MockJournalConfiguration> journalConfig;
    journalLogBuffer.Init(&journalConfig, nullptr, 0, nullptr);

    // When
    uint64_t groupSize = 1024;
    EXPECT_CALL(journalConfig, GetLogGroupSize).WillOnce(Return(groupSize));
    journalLogBuffer.InitDataBuffer();

    // Then: InitializedDataBuffer will be created
    char* actual = journalLogBuffer.GetInitializedDataBuffer();
    EXPECT_EQ(true, actual != nullptr);
}

TEST(JournalLogBuffer, Dispose_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);
    MockJournalConfiguration journalConfig;
    journalLogBuffer.Init(&journalConfig, nullptr, 0, nullptr);

    // When, Then
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, Close).WillOnce(Return(0));
    journalLogBuffer.Dispose();
}

TEST(JournalLogBuffer, Dispose_testIfLogFileIsNotOpened)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When, Then
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(false));
    journalLogBuffer.Dispose();
}

TEST(JournalLogBuffer, Dispose_testIfLogFileIsNotClosed)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When, Then
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, Close).WillOnce(Return(-1));
    journalLogBuffer.Dispose();
}

TEST(JournalLogBuffer, Create_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When, Then
    uint64_t logBufferSize = 1024;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Create(logBufferSize)).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(0));
    journalLogBuffer.Create(logBufferSize);
}

TEST(JournalLogBuffer, Create_testIfLogBufferAlreadyExisted)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    uint64_t logBufferSize = 1024;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    int result = journalLogBuffer.Create(logBufferSize);

    // Then: journalLogBuffer will be return error code
    int retCode = -1 * EID(JOURNAL_LOG_BUFFER_CREATE_FAILED);
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, Create_testIfLogBufferCreateFailed)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    uint64_t logBufferSize = 1024;
    int retCode = -1;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Create(logBufferSize)).WillOnce(Return(retCode));
    int result = journalLogBuffer.Create(logBufferSize);

    // Then: journalLogBuffer will be return error code
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, Create_testIfLogBufferOpenFailed)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    uint64_t logBufferSize = 1024;
    int retCode = -1;
    EXPECT_CALL(*metaFile, DoesFileExist()).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Create(logBufferSize)).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(retCode));
    int result = journalLogBuffer.Create(logBufferSize);

    // Then: journalLogBuffer will be return error code
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, Open_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    uint64_t logBufferSize = 1024;
    int retCode = 0;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(retCode));
    EXPECT_CALL(*metaFile, GetFileSize).WillOnce(Return(logBufferSize));

    uint64_t actual;
    int result = journalLogBuffer.Open(actual);

    // Then
    EXPECT_EQ(retCode, result);
    EXPECT_EQ(logBufferSize, actual);
}

TEST(JournalLogBuffer, Open_testIfLogBufferFileDoesNotExist)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    uint64_t logBufferSize;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(false));
    int result = journalLogBuffer.Open(logBufferSize);

    // Then: journalLogBuffer will be return error code
    int retCode = -1 * EID(JOURNAL_LOG_BUFFER_OPEN_FAILED);
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, Open_testIfLogBufferFileOpenFailed)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    uint64_t logBufferSize;
    int retCode = -1;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(retCode));
    int result = journalLogBuffer.Open(logBufferSize);

    // Then: journalLogBuffer will be return error code
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, ReadLogBuffer_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockJournalConfiguration> journalConfig;
    JournalLogBuffer journalLogBuffer(metaFile);
    journalLogBuffer.Init(&journalConfig, nullptr, 0, nullptr);

    // When
    uint64_t logGroupSize = 1024;
    int logGroupId = 1;
    int retCode = 0;
    EXPECT_CALL(journalConfig, GetLogGroupSize).WillRepeatedly(Return(logGroupSize));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce([&](AsyncMetaFileIoCtx* ctx)
                {
                    journalLogBuffer.SetLogBufferReadDone(true);
                    delete ctx;
                    return retCode;
                });
    int result = journalLogBuffer.ReadLogBuffer(logGroupId, nullptr);

    // Then
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, ReadLogBuffer_testIfReadFail)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockJournalConfiguration> journalConfig;
    JournalLogBuffer journalLogBuffer(metaFile);
    journalLogBuffer.Init(&journalConfig, nullptr, 0, nullptr);

    // When
    uint64_t logGroupSize = 1024;
    int logGroupId = 1;
    int retCode = -1;
    EXPECT_CALL(journalConfig, GetLogGroupSize).WillRepeatedly(Return(logGroupSize));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(retCode));
    int result = journalLogBuffer.ReadLogBuffer(logGroupId, nullptr);

    // Then: journalLogBuffer will be return error code
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, WriteLog_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockJournalConfiguration> journalConfig;
    NiceMock<MockLogBufferIoContextFactory> ioContextFactory;
    NiceMock<MockTelemetryPublisher> telemetry;

    JournalLogBuffer journalLogBuffer(metaFile);
    journalLogBuffer.Init(&journalConfig, &ioContextFactory, 0, &telemetry);

    NiceMock<MockLogWriteContext> context;
    auto callback = [](AsyncMetaFileIoCtx* ctx) {};

    // When
    NiceMock<MockMapUpdateLogWriteContext> ioContext;
    EXPECT_CALL(ioContextFactory, CreateMapUpdateLogWriteIoContext).WillOnce(Return(&ioContext));
    EXPECT_CALL(*metaFile, GetFd).WillOnce(Return(1));
    int retCode = 0;
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(retCode));

    int result = journalLogBuffer.WriteLog(&context, 0, callback);

    // Then: journalLogBuffer will be return error code
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, WriteLog_testIfWriteFailed)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockJournalConfiguration> journalConfig;
    NiceMock<MockLogBufferIoContextFactory> ioContextFactory;
    NiceMock<MockTelemetryPublisher> telemetry;

    JournalLogBuffer journalLogBuffer(metaFile);
    journalLogBuffer.Init(&journalConfig, &ioContextFactory, 0, &telemetry);

    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    auto callback = [](AsyncMetaFileIoCtx* ctx) {};

    // When
    NiceMock<MockMapUpdateLogWriteContext>* ioContext = new NiceMock<MockMapUpdateLogWriteContext>;
    EXPECT_CALL(ioContextFactory, CreateMapUpdateLogWriteIoContext).WillOnce(Return(ioContext));
    EXPECT_CALL(*metaFile, GetFd).WillOnce(Return(1));
    int retCode = -1;
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(retCode));

    int result = journalLogBuffer.WriteLog(context, 0, callback);

    // Then
    retCode = -1 * EID(JOURNAL_LOG_WRITE_FAILED);
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, SyncResetAll_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockJournalConfiguration> journalConfig;
    NiceMock<MockLogBufferIoContextFactory> ioContextFactory;
    NiceMock<MockTelemetryPublisher> telemetry;

    JournalLogBuffer journalLogBuffer(metaFile);
    journalLogBuffer.Init(&journalConfig, &ioContextFactory, 0, &telemetry);

    // When: Async reset request as much as number of log groups, and Reset completed successfully
    int numLogGroups = 2;
    uint64_t logGroupSize = 1024;
    EXPECT_CALL(journalConfig, GetNumLogGroups).WillRepeatedly(Return(numLogGroups));
    EXPECT_CALL(journalConfig, GetLogGroupSize).WillRepeatedly(Return(logGroupSize));

    int fd = 1;
    int retCode = 0;
    std::vector<NiceMock<MockLogBufferIoContext>*> resetContextList;
    for (int id = 0; id < numLogGroups; id++)
    {
        NiceMock<MockLogBufferIoContext>* resetRequest = new NiceMock<MockLogBufferIoContext>;
        resetContextList.push_back(resetRequest);
        EXPECT_CALL(ioContextFactory, CreateLogBufferIoContext(id, _)).WillRepeatedly(Return(resetRequest));
        EXPECT_CALL(*metaFile, GetFd).WillRepeatedly(Return(fd));
        EXPECT_CALL(*metaFile, AsyncIO).WillRepeatedly([&](AsyncMetaFileIoCtx* ctx)
            {
                journalLogBuffer.LogGroupResetCompleted(id);
                return retCode;
            });
    }

    int result = journalLogBuffer.SyncResetAll();

    // Then
    EXPECT_EQ(retCode, result);

    for (auto resetContext : resetContextList)
    {
        delete resetContext;
    }
}

TEST(JournalLogBuffer, SyncResetAll_testIfAsyncIOFailed)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockJournalConfiguration> journalConfig;
    NiceMock<MockLogBufferIoContextFactory> ioContextFactory;
    NiceMock<MockTelemetryPublisher> telemetry;

    JournalLogBuffer journalLogBuffer(metaFile);
    journalLogBuffer.Init(&journalConfig, &ioContextFactory, 0, &telemetry);

    // When: Async reset failed
    int numLogGroups = 2;
    uint64_t logGroupSize = 1024;
    EXPECT_CALL(journalConfig, GetNumLogGroups).WillRepeatedly(Return(numLogGroups));
    EXPECT_CALL(journalConfig, GetLogGroupSize).WillRepeatedly(Return(logGroupSize));

    int fd = 1;
    int retCode = -1;
    int groupId = 0;

    EXPECT_CALL(ioContextFactory, CreateLogBufferIoContext(groupId, _)).WillRepeatedly(Return(new NiceMock<MockLogBufferIoContext>));
    EXPECT_CALL(*metaFile, GetFd).WillRepeatedly(Return(fd));
    EXPECT_CALL(*metaFile, AsyncIO).WillRepeatedly([&](AsyncMetaFileIoCtx* ctx)
        {
            journalLogBuffer.LogGroupResetCompleted(groupId);
            return retCode;
        });

    int result = journalLogBuffer.SyncResetAll();

    // Then
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, WriteLogGroupFooter_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockJournalConfiguration> journalConfig;
    NiceMock<MockLogBufferIoContextFactory> ioContextFactory;
    NiceMock<MockTelemetryPublisher> telemetry;

    JournalLogBuffer journalLogBuffer(metaFile);
    journalLogBuffer.Init(&journalConfig, &ioContextFactory, 0, &telemetry);

    int groupId = 0;
    NiceMock<CallbackSmartPtr> callback;

    // When
    int fd = 1;
    int retCode = 0;
    uint64_t footerOffset = 10;
    LogGroupFooter footer;
    footer.isReseted = false;
    footer.lastCheckpointedSeginfoVersion = 3;
    footer.resetedSequenceNumber = 4;

    NiceMock<MockLogBufferIoContext> resetRequest;
    EXPECT_CALL(ioContextFactory, CreateLogGroupFooterWriteContext).WillOnce(Return(&resetRequest));
    EXPECT_CALL(*metaFile, GetFd).WillOnce(Return(fd));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(retCode));

    int result = journalLogBuffer.WriteLogGroupFooter(footerOffset, footer, groupId, callback);

    // Then
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, Delete_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    int retCode = 0;
    EXPECT_CALL(*metaFile, Delete).WillOnce(Return(retCode));

    bool isExist = true;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(isExist));
    int result = journalLogBuffer.Delete();

    // Then
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, Delete_testIfDeleteFailed)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    int retCode = -1;
    EXPECT_CALL(*metaFile, Delete).WillOnce(Return(retCode));

    bool isExist = true;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(isExist));
    int result = journalLogBuffer.Delete();

    // Then
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, Delete_testIfFileDoesNotExist)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    int retCode = 0;

    bool isExist = false;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(isExist));
    int result = journalLogBuffer.Delete();

    // Then
    EXPECT_EQ(retCode, result);
}

TEST(JournalLogBuffer, DoesLogFileExist_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    JournalLogBuffer journalLogBuffer(metaFile);

    // When
    bool expect = true;
    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(expect));
    int result = journalLogBuffer.DoesLogFileExist();

    // Then
    EXPECT_EQ(expect, result);
}
} // namespace pos
