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

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    EXPECT_CALL(client, GetNumSections).WillRepeatedly(Return(NUM_SEGMENT_CTX_SECTION));

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
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    EXPECT_CALL(client, GetNumSections).WillRepeatedly(Return(NUM_SEGMENT_CTX_SECTION));

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
    NiceMock<MockMetaFileIntf> file("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(file, Create).WillOnce(Return(0));
    EXPECT_CALL(file, Open);

    int ret = fileManager.LoadContext();
    EXPECT_EQ(ret, 0);
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

    int ret = fileManager.LoadContext();
    EXPECT_EQ(ret, 1);
    EXPECT_EQ(fileManager.GetNumFilesReading(), 1);
}

TEST(AllocatorFileIo, LoadContext_testFileLoadFail)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf> file("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, &file);

    EXPECT_CALL(file, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(file, AsyncIO).WillOnce(Return(-1));

    int ret = fileManager.LoadContext();
    EXPECT_TRUE(ret < 0);
}

TEST(AllocatorFileIo, LoadContext_testLoadAndCallback)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    // Prepare loaded buffer for test
    EXPECT_CALL(client, GetNumSections).WillRepeatedly(Return(NUM_SEGMENT_CTX_SECTION));
    EXPECT_CALL(client, GetSignature).WillRepeatedly(Return(0xAFAFAFAF));

    char* buffer[NUM_SEGMENT_CTX_SECTION];

    buffer[0] = (char*)(new CtxHeader());
    EXPECT_CALL(client, GetSectionAddr(0)).WillOnce(Return(buffer[0]));
    EXPECT_CALL(client, GetSectionSize(0)).WillOnce(Return(sizeof(CtxHeader)));

    for (int sectionId = 1; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        buffer[sectionId] = new char[100]();
        EXPECT_CALL(client, GetSectionAddr(sectionId)).WillOnce(Return(buffer[sectionId]));
        EXPECT_CALL(client, GetSectionSize(sectionId)).WillOnce(Return(100));
    }

    fileManager.Init();

    EXPECT_CALL(*file, AsyncIO)
        .WillOnce([&](AsyncMetaFileIoCtx* ctx)
        {
            // Update the buffer with the prepared buffer above
            EXPECT_EQ(fileManager.GetNumFilesReading(), 1);

            CtxHeader header = {
                .sig = 0xAFAFAFAF,
                .ctxVersion = 0 };
            memcpy(ctx->buffer, &header, sizeof(header));

            char* ptr = ctx->buffer + sizeof(header);
            for (int sectionId = 1; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
            {
                memset(ptr + 100 * (sectionId - 1), 'a' + sectionId, 100);
            }

            ctx->ioDoneCheckCallback = [](void* ctx) { return 0; };
            ctx->HandleIoComplete(ctx);

            return 0;
        });

    // Load Context
    EXPECT_CALL(*file, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(client, AfterLoad);

    int ret = fileManager.LoadContext();
    EXPECT_EQ(ret, 1);

    EXPECT_EQ(fileManager.GetNumFilesReading(), 0);

    // Compare buffer is loaded with expected data
    EXPECT_EQ(((CtxHeader*)buffer[0])->sig, 0xAFAFAFAF);
    EXPECT_EQ(((CtxHeader*)buffer[0])->ctxVersion, 0);
    delete[] buffer[0];

    for (int sectionId = 1; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        EXPECT_EQ(*(buffer[sectionId]), 'a' + sectionId);

        delete[] buffer[sectionId];
    }
}

TEST(AllocatorFileIo, Flush_testFlushAndCallback)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(ALLOCATOR_CTX, &client, &addrInfo, file);

    // Prepare buffer to flush
    EXPECT_CALL(client, GetNumSections).WillRepeatedly(Return(NUM_SEGMENT_CTX_SECTION));
    EXPECT_CALL(client, GetSignature).WillRepeatedly(Return(0xAFAFAFAF));

    char* buffer[NUM_SEGMENT_CTX_SECTION];

    buffer[0] = (char*)(new CtxHeader());
    ((CtxHeader*)buffer[0])->sig = 0xAFAFAFAF;
    ((CtxHeader*)buffer[0])->ctxVersion = 0;

    EXPECT_CALL(client, GetSectionAddr(0)).WillOnce(Return(buffer[0]));
    EXPECT_CALL(client, GetSectionSize(0)).WillOnce(Return(sizeof(CtxHeader)));

    for (int sectionId = 1; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        buffer[sectionId] = new char[100]();
        memset(buffer[sectionId], 'a' + sectionId, 100);
        EXPECT_CALL(client, GetSectionAddr(sectionId)).WillOnce(Return(buffer[sectionId]));
        EXPECT_CALL(client, GetSectionSize(sectionId)).WillOnce(Return(100));
    }
    fileManager.Init();

    std::mutex lock;
    EXPECT_CALL(client, GetCtxLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(client, FinalizeIo);
    EXPECT_CALL(client, BeforeFlush).Times(1);

    EXPECT_CALL(*file, AsyncIO)
        .WillOnce([&](AsyncMetaFileIoCtx* ctx)
        {
            EXPECT_EQ(fileManager.GetNumFilesFlushing(), 1);

            EXPECT_EQ(((CtxHeader*)ctx->buffer)->sig, 0xAFAFAFAF);
            EXPECT_EQ(((CtxHeader*)ctx->buffer)->ctxVersion, 0);

            char* ptr = ctx->buffer + sizeof(CtxHeader);
            for (int sectionId = 1; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
            {
                EXPECT_EQ(*ptr, 'a' + sectionId);
                ptr += 100;
            }

            ctx->ioDoneCheckCallback = [](void* ctx) { return 0; };
            ctx->HandleIoComplete(ctx);

            return 0;
        });

    auto testCallback = []() {}; // do nothing
    const int INVALID_SECTION_ID = -1;
    int ret = fileManager.Flush(testCallback, INVALID_SECTION_ID);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(fileManager.GetNumFilesFlushing(), 0);

    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        delete[] buffer[sectionId];
    }
}

TEST(AllocatorFileIo, Flush_testFlushFail)
{
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    EXPECT_CALL(client, GetNumSections).WillRepeatedly(Return(NUM_SEGMENT_CTX_SECTION));

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

    auto testCallback = []() {}; // do nothing
    const int INVALID_SECTION_ID = -1;
    int ret = fileManager.Flush(testCallback, INVALID_SECTION_ID);
    EXPECT_TRUE(ret < 0);

    for (int sectionId = 0; sectionId < NUM_SEGMENT_CTX_SECTION; sectionId++)
    {
        delete[] buffer[sectionId];
    }
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

TEST(AllocatorFileIo, GetSectionAddr_TestSimpleGetter)
{
    // given
    NiceMock<MockIAllocatorFileIoClient> client;
    NiceMock<AllocatorAddressInfo> addrInfo;
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    char* buffer[NUM_SEGMENT_CTX_SECTION];
    EXPECT_CALL(client, GetNumSections).WillRepeatedly(Return(NUM_SEGMENT_CTX_SECTION));

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
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb", MetaFileType::Map);
    AllocatorFileIo fileManager(SEGMENT_CTX, &client, &addrInfo, file);

    EXPECT_CALL(client, GetNumSections).WillRepeatedly(Return(NUM_SEGMENT_CTX_SECTION));
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
