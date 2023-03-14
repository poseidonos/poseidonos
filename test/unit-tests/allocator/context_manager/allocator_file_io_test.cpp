#include "src/allocator/context_manager/allocator_file_io.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/i_allocator_file_io_client_mock.h"
#include "test/unit-tests/meta_file_intf/async_context_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"
#include "src/include/pos_event_id.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/include/allocator_const.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(AllocatorFileIo, AllocatorFileIo_testConstructor)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;

    {
        AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo);
    }

    {
        NiceMock<MockMetaFileIntf> file("aa", "bb", MetaFileType::Map);
        AllocatorFileIo fileManager(ALLOCATOR_CTX, &client, &addrInfo, &file);
    }
}

TEST(AllocatorFileIo, Init_TestInitAndClose)
{
    // given
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    // when
    fileManager.Init();

    EXPECT_CALL(*file, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*file, Close);
    fileManager.Dispose();
}

TEST(AllocatorFileIo, Init_TestInitAndCloseAndFileNotOpened)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    fileManager.Init();
    EXPECT_CALL(*file, IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*file, Close).Times(0);
    fileManager.Dispose();
}

TEST(AllocatorFileIo, LoadContext_testFileNotExist)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf> file("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(file, Create).WillOnce(Return(0));
    EXPECT_CALL(file, Open);

    int ret = fileManager.LoadContext();
    EXPECT_EQ(ret, EID(SUCCEED_TO_OPEN_WITH_CREATION));
}

TEST(AllocatorFileIo, LoadContext_testFileCreationFail)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf> file("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(file, Create).WillOnce(Return(-1));

    int ret = fileManager.LoadContext();
    EXPECT_TRUE(ret < 0);
}

TEST(AllocatorFileIo, LoadContext_testFileLoad)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf> file("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(file, AsyncIO).WillOnce(Return(0));
    EXPECT_CALL(file, Open).WillOnce(Return(0));

    int ret = fileManager.LoadContext();
    EXPECT_EQ(ret, EID(SUCCEED_TO_OPEN_WITHOUT_CREATION));
    EXPECT_EQ(fileManager.GetNumOutstandingRead(), 1);
}

TEST(AllocatorFileIo, LoadContext_testFileLoadFail)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf> file("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(file, AsyncIO).WillOnce(Return(-1));
    EXPECT_CALL(file, Open).WillOnce(Return(0));

    int ret = fileManager.LoadContext();
    EXPECT_TRUE(ret < 0);
}

TEST(AllocatorFileIo, LoadContext_testLoadAndCallback)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    uint64_t testDataSize = sizeof(SegmentCtxHeader) + 100;
    ON_CALL(client, GetTotalDataSize).WillByDefault(Return(testDataSize));
    
    char expectedBuffer[testDataSize];
    memset((void*)expectedBuffer, 0, testDataSize);
    SegmentCtxHeader* headerPtr = reinterpret_cast<SegmentCtxHeader*>(expectedBuffer);
    headerPtr->sig = SIG_SEGMENT_CTX;
    headerPtr->ctxVersion = 3;

    EXPECT_CALL(*file, GetIoDoneCheckFunc).WillOnce(Return([](void* ctx) { return 0; }));
    fileManager.Init();

    EXPECT_CALL(*file, AsyncIO)
        .WillOnce([&](AsyncMetaFileIoCtx* ctx)
        {
            EXPECT_EQ(fileManager.GetNumOutstandingRead(), 1);

            // Update the buffer with the prepared buffer above
            memset((void*)ctx->GetBuffer(), 0, testDataSize);
            SegmentCtxHeader* headerPtr = reinterpret_cast<SegmentCtxHeader*>(ctx->GetBuffer());
            headerPtr->sig = SIG_SEGMENT_CTX;
            headerPtr->ctxVersion = 3;

            ctx->HandleIoComplete(ctx);

            return 0;
        });

    // Load Context
    EXPECT_CALL(*file, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(client, AfterLoad).WillOnce([&](char* buf) {
        // Compare buffer is loaded with expected data
        EXPECT_EQ(memcmp(expectedBuffer, buf, testDataSize), 0);
    });

    int ret = fileManager.LoadContext();
    EXPECT_EQ(ret, EID(SUCCEED_TO_OPEN_WITHOUT_CREATION));

    EXPECT_EQ(fileManager.GetNumOutstandingRead(), 0);
}

TEST(AllocatorFileIo, Flush_testFlushAndCallback)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);
    fileManager.Init();

    // Prepare buffer to flush
    uint64_t testDataSize = sizeof(SegmentCtxHeader) + 1000;
    ON_CALL(client, GetTotalDataSize).WillByDefault(Return(testDataSize));
    
    char expectedBuffer[testDataSize];
    memset((void*)expectedBuffer, 0, testDataSize);
    SegmentCtxHeader* headerPtr = reinterpret_cast<SegmentCtxHeader*>(expectedBuffer);
    headerPtr->sig = SIG_SEGMENT_CTX;
    headerPtr->ctxVersion = 5;

    EXPECT_CALL(client, BeforeFlush)
        .WillOnce([&](char* buf, ContextSectionBuffer buffer) {
            memset(buf, 0, testDataSize);
            SegmentCtxHeader* headerPtr = reinterpret_cast<SegmentCtxHeader*>(buf);
            headerPtr->sig = SIG_SEGMENT_CTX;
            headerPtr->ctxVersion = 5;
        });
    EXPECT_CALL(client, AfterFlush);

    EXPECT_CALL(*file, GetIoDoneCheckFunc).WillOnce(Return([](void* ctx) { return 0; }));
    EXPECT_CALL(*file, AsyncIO)
        .WillOnce([&](AsyncMetaFileIoCtx* ctx) {
            // Data to be flushed
            EXPECT_EQ(fileManager.GetNumOutstandingFlush(), 1);
            EXPECT_EQ(memcmp(ctx->GetBuffer(), expectedBuffer, testDataSize), 0);

            ctx->HandleIoComplete(ctx);

            return 0;
        });

    auto testCallback = []() {}; // do nothing
    int ret = fileManager.Flush(testCallback);
    EXPECT_EQ(ret, 0);
}

TEST(AllocatorFileIo, Flush_testFlushFail)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);
    fileManager.Init();

    ON_CALL(client, GetTotalDataSize).WillByDefault(Return(10)); // dummy 

    EXPECT_CALL(client, BeforeFlush);
    EXPECT_CALL(*file, AsyncIO).WillOnce(Return(-1));

    auto testCallback = []() {}; // do nothing
    int ret = fileManager.Flush(testCallback);
    EXPECT_TRUE(ret < 0);
}

TEST(AllocatorFileIo, GetStoredVersion_TestSimpleGetter)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    EXPECT_CALL(client, GetStoredVersion).WillOnce(Return(20));

    int ret = fileManager.GetStoredVersion();
    EXPECT_EQ(ret, 20);

    delete file;
}

TEST(AllocatorFileIo, GetSectionInfo_TestSimpleGetter)
{
    // given
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    int sectionId = 3;
    ContextSectionAddr info = {
        .offset = 0,
        .size = 30
    };
    EXPECT_CALL(client, GetSectionInfo(sectionId)).WillOnce(Return(info));

    auto actual = fileManager.GetSectionSize(sectionId);
    EXPECT_EQ(actual, info.size);
}

} // namespace pos
