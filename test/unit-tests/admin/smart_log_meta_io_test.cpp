#include "src/admin/smart_log_meta_io.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/admin/smart_log_mgr_mock.h"
#include "test/unit-tests/admin/smart_log_meta_io_mock.h"
#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"
using ::testing::_;
using testing::NiceMock;
using testing::Return;
using ::testing::SetArgPointee;
namespace pos
{
TEST(MetaIoDoneChecker, SetReady_testIfTheInternalFlagIsCleared)
{
    MetaIoDoneChecker doneChecker;
    doneChecker.SetReady();

    EXPECT_FALSE(doneChecker.IsDone());
}

TEST(MetaIoDoneChecker, SetReady_testIfTheInternalFlagIsSet)
{
    MetaIoDoneChecker doneChecker;
    doneChecker.SetDone();

    EXPECT_TRUE(doneChecker.IsDone());
}

TEST(SmartLogMetaIo, SmartLogMetaIo_Constructor_Stack_Three_Arguments)
{
    uint32_t arrayIndex = 0;
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);
}

TEST(SmartLogMetaIo, SmartLogMetaIo_Constructor_One_Stack)
{
    uint32_t arrayIndex = 0;
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr);
}

TEST(SmartLogMetaIo, SmartLogMetaIo_Constructor_One_Heap)
{
    uint32_t arrayIndex = 0;
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    SmartLogMetaIo* smartLogMetaIo = new SmartLogMetaIo(arrayIndex, &mockSmartLogMgr);
    delete smartLogMetaIo;
}

TEST(SmartLogMetaIo, Init_SmartLogEnabledFalse)
{
    uint32_t arrayIndex = 0;

    NiceMock<MockSmartLogMgr>* mockSmartLogMgr = new NiceMock<MockSmartLogMgr>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, mockSmartLogMgr);
    ON_CALL(*mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(false));
    ON_CALL(*mockSmartLogMgr, Init()).WillByDefault(Return());

    int expected = 0;
    int actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
    delete mockSmartLogMgr;
}

TEST(SmartLogMetaIo, Init_SmartLogEnabledTrue)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    EXPECT_CALL(mockSmartLogMgr, Init).Times(1);
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));
    uint32_t arrayIndex = 0;
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    uint64_t fileSize = 1024;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);
    int expected = 0;
    int actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
}

TEST(SmartLogMetaIo, Init_CreateFileCreationSuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;

    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Create(_)).WillOnce(Return(0));
    int expected = 0;
    int actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
}

TEST(SmartLogMetaIo, Init_CreateFileCreationUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Create(_)).WillOnce(Return(-1));
    int expected = -1;
    int actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
}

TEST(SmartLogMetaIo, Init_FileLoad_OpenedFile_AsyncIOSuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    ON_CALL(*doneChecker, SetReady).WillByDefault(Return());
    ON_CALL(*doneChecker, IsDone).WillByDefault(Return(true));
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, Close).WillOnce(Return(0));
    int expected = 0;
    int actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
}

TEST(SmartLogMetaIo, Init_FileLoad_OpenedFile_AsyncIOSuccessful_Close_Unsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    ON_CALL(*doneChecker, SetReady).WillByDefault(Return());
    ON_CALL(*doneChecker, IsDone).WillByDefault(Return(true));
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    AsyncMetaFileIoCtx *ctx = new AsyncMetaFileIoCtx();
    ctx->error = 0;
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce([&](AsyncMetaFileIoCtx* ctx)
               {
                   smartLogMetaIo.DeleteAsyncIoCtx(ctx);
                   return 0;
               });

    EXPECT_CALL(*metaFile, Close).WillOnce(Return(2));
    int expected = 2;
    int actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
}

TEST(SmartLogMetaIo, Init_FileLoad_OpenedFile_AsyncIOUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(-1));
    int expected = -1;
    int actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
}

TEST(SmartLogMetaIo, Init_FileLoad_UnopenedFile_AsyncIOsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    ON_CALL(*doneChecker, SetReady).WillByDefault(Return());
    ON_CALL(*doneChecker, IsDone).WillByDefault(Return(true));
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    int expected = 0;
    int actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
}

TEST(SmartLogMetaIo, Init_FileLoad_UnopenedFile_AsyncIOUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(-1));
    int expected = -1;
    int actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
}

TEST(SmartLogMetaIo, Dispose_SmartLogEnabledFalse)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(false));

    uint32_t arrayIndex = 0;
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;

    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);
    smartLogMetaIo.Dispose();
}

TEST(SmartLogMetaIo, Dispose_SmartLogEnabledTrue)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;

    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    ON_CALL(*doneChecker, SetReady).WillByDefault(Return());
    ON_CALL(*doneChecker, IsDone).WillByDefault(Return(true));
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);
    smartLogMetaIo.Dispose();
}

TEST(SmartLogMetaIo, Dispose_FileOpenedAsyncIOSuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    ON_CALL(*doneChecker, SetReady).WillByDefault(Return());
    ON_CALL(*doneChecker, IsDone).WillByDefault(Return(true));
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    smartLogMetaIo.Dispose();
}

TEST(SmartLogMetaIo, Dispose_FileOpenedAsyncIOUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    ON_CALL(*doneChecker, SetReady).WillByDefault(Return());
    ON_CALL(*doneChecker, IsDone).WillByDefault(Return(true));
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(-1));
    smartLogMetaIo.Dispose();
}

TEST(SmartLogMetaIo, Dispose_FileUnopenedAsyncIOSuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    ON_CALL(*doneChecker, SetReady).WillByDefault(Return());
    ON_CALL(*doneChecker, IsDone).WillByDefault(Return(true));
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    smartLogMetaIo.Dispose();
}

TEST(SmartLogMetaIo, Dispose_FileUnopenedAsyncIOUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    ON_CALL(*doneChecker, SetReady).WillByDefault(Return());
    ON_CALL(*doneChecker, IsDone).WillByDefault(Return(true));
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);

    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    smartLogMetaIo.Dispose();
}

TEST(SmartLogMetaIo, Shutdown_Run)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);
    smartLogMetaIo.Shutdown();
}

TEST(SmartLogMetaIo, Flush_Run)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    NiceMock<MockMetaIoDoneChecker>* doneChecker = new NiceMock<MockMetaIoDoneChecker>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile, doneChecker);
    smartLogMetaIo.Flush();
}

} // namespace pos
