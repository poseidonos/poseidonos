#include "src/allocator/context_manager/allocator_file_io.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/i_allocator_file_io_client_mock.h"
#include "test/unit-tests/meta_file_intf/async_context_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"

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
        AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, 0);
    }

    {
        NiceMock<MockMetaFileIntf> file("aa", "bb");
        AllocatorFileIo fileManager(ALLOCATOR_CTX, &client, &addrInfo, &file);
    }
}

TEST(AllocatorFileIo, Init_TestInitAndClose)
{
    // given
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        buffer[sectionId] = new char[100 * sectionId]();
        memset(buffer[sectionId], 'a' + sectionId, 100 * sectionId);

        EXPECT_CALL(client, GetSectionSize(sectionId)).WillRepeatedly(Return(sectionId * 100));
        EXPECT_CALL(client, GetSectionAddr(sectionId)).WillRepeatedly(Return(buffer[sectionId]));
    }

    // when
    fileManager.Init();

    // then
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        EXPECT_EQ(fileManager.GetSectionSize(sectionId), sectionId * 100);
        EXPECT_EQ(fileManager.GetSectionAddr(sectionId), buffer[sectionId]);
    }

    EXPECT_CALL(*file, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*file, Close);
    fileManager.Dispose();

    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        delete[] buffer[sectionId];
    }
}

TEST(AllocatorFileIo, Init_TestInitAndCloseAndFileNotOpened)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        buffer[sectionId] = new char[100 * sectionId]();
        memset(buffer[sectionId], 'a' + sectionId, 100 * sectionId);

        EXPECT_CALL(client, GetSectionSize(sectionId)).WillRepeatedly(Return(sectionId * 100));
        EXPECT_CALL(client, GetSectionAddr(sectionId)).WillRepeatedly(Return(buffer[sectionId]));
    }

    fileManager.Init();
    EXPECT_CALL(*file, IsOpened).WillOnce(Return(false));
    fileManager.Dispose();

    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        delete[] buffer[sectionId];
    }
}

TEST(AllocatorFileIo, LoadContext_testFileNotExist)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf> file("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(file, Create).WillOnce(Return(0));
    EXPECT_CALL(file, Open);

    int ret = fileManager.LoadContext(nullptr);
    EXPECT_EQ(ret, 0);
}

TEST(AllocatorFileIo, LoadContext_testFileCreationFail)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf> file("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(file, Create).WillOnce(Return(-1));

    int ret = fileManager.LoadContext(nullptr);
    EXPECT_TRUE(ret < 0);
}

TEST(AllocatorFileIo, LoadContext_testFileLoad)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf> file("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(file, AsyncIO).WillOnce(Return(0));

    int ret = fileManager.LoadContext(nullptr);
    EXPECT_EQ(ret, 1);
}

TEST(AllocatorFileIo, LoadContext_testFileLoadFail)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf> file("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(file, AsyncIO).WillOnce(Return(-1));

    int ret = fileManager.LoadContext(nullptr);
    EXPECT_TRUE(ret < 0);
}

TEST(AllocatorFileIo, AfterLoad_testMemoryLoaded)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    EXPECT_CALL(client, GetSectionSize).WillRepeatedly(Return(100));

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        buffer[sectionId] = new char[100]();
        EXPECT_CALL(client, GetSectionAddr(sectionId)).WillOnce(Return(buffer[sectionId]));
        EXPECT_CALL(client, GetSectionSize(sectionId)).WillOnce(Return(100));
    }

    fileManager.Init();

    EXPECT_CALL(client, AfterLoad);

    char* loadedData = new char[100 * NUM_SEGMENT_CTX_SECTION]();
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        memset(loadedData + 100 * sectionId, 'a' + sectionId, 100);
    }

    fileManager.AfterLoad(loadedData);

    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        EXPECT_EQ(*(buffer[sectionId]), 'a' + sectionId);

        delete[] buffer[sectionId];
    }
    delete[] loadedData;
}

TEST(AllocatorFileIo, Flush_segmentCtx)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        buffer[sectionId] = new char[100]();
        memset(buffer[sectionId], 'a' + sectionId, 100);
        EXPECT_CALL(client, GetSectionAddr(sectionId)).WillOnce(Return(buffer[sectionId]));
    }
    fileManager.Init();

    std::mutex lock;
    EXPECT_CALL(client, GetCtxLock).WillOnce(ReturnRef(lock));

    EXPECT_CALL(client, BeforeFlush);
    EXPECT_CALL(*file, AsyncIO).WillOnce(Return(0));

    int ret = fileManager.Flush(nullptr);
    EXPECT_EQ(ret, 0);

    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        delete[] buffer[sectionId];
    }
}

TEST(AllocatorFileIo, Flush_allocatorCtx)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(ALLOCATOR_CTX, &client, &addrInfo, file);

    char* buffer[NUM_ALLOCATOR_CTX_SECTION];
    for (int sectionId = 0; sectionId < NUM_ALLOCATOR_CTX_SECTION; sectionId++)
    {
        buffer[sectionId] = new char[100]();
        memset(buffer[sectionId], 'a' + sectionId, 100);
        EXPECT_CALL(client, GetSectionAddr(sectionId)).WillOnce(Return(buffer[sectionId]));
    }
    fileManager.Init();

    std::mutex lock;
    EXPECT_CALL(client, GetCtxLock).WillOnce(ReturnRef(lock));

    EXPECT_CALL(client, BeforeFlush).Times(1);
    EXPECT_CALL(*file, AsyncIO).WillOnce(Return(0));

    int ret = fileManager.Flush(nullptr);
    EXPECT_EQ(ret, 0);

    for (int sectionId = 0; sectionId < NUM_ALLOCATOR_CTX_SECTION; sectionId++)
    {
        delete[] buffer[sectionId];
    }
}

TEST(AllocatorFileIo, Flush_testFlushFail)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        buffer[sectionId] = new char[100]();
        memset(buffer[sectionId], 'a' + sectionId, 100);
        EXPECT_CALL(client, GetSectionAddr(sectionId)).WillOnce(Return(buffer[sectionId]));
    }
    fileManager.Init();

    std::mutex lock;
    EXPECT_CALL(client, GetCtxLock).WillOnce(ReturnRef(lock));

    EXPECT_CALL(client, BeforeFlush);
    EXPECT_CALL(*file, AsyncIO).WillOnce(Return(-1));

    int ret = fileManager.Flush(nullptr);
    EXPECT_TRUE(ret < 0);

    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        delete[] buffer[sectionId];
    }
}

TEST(AllocatorFileIo, AfterFlush_SimpleTest)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    NiceMock<MockAsyncMetaFileIoCtx> ctx;
    EXPECT_CALL(client, FinalizeIo(&ctx));

    fileManager.AfterFlush(&ctx);
}

TEST(AllocatorFileIo, GetStoredVersion_TestSimpleGetter)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    EXPECT_CALL(client, GetStoredVersion).WillOnce(Return(20));

    int ret = fileManager.GetStoredVersion();
    EXPECT_EQ(ret, 20);
}

TEST(AllocatorFileIo, GetSectionAddr_TestSimpleGetter)
{
    // given
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        buffer[sectionId] = new char[100]();
        memset(buffer[sectionId], 'a' + sectionId, 100);
        EXPECT_CALL(client, GetSectionAddr(sectionId)).WillOnce(Return(buffer[sectionId]));
    }
    fileManager.Init();

    // when
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        char* ret = fileManager.GetSectionAddr(sectionId);
        EXPECT_EQ(ret, buffer[sectionId]);
    }

    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        delete[] buffer[sectionId];
    }
}

TEST(AllocatorFileIo, GetSectionSize_TestSimpleGetter)
{
    // given
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        EXPECT_CALL(client, GetSectionSize(sectionId)).WillOnce(Return(sectionId * 100));
    }
    fileManager.Init();

    // when
    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        int ret = fileManager.GetSectionSize(sectionId);
        EXPECT_EQ(ret, sectionId * 100);
    }
}
} // namespace pos
