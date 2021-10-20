#include "src/admin/smart_log_meta_io.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/admin/smart_log_mgr_mock.h"
#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"
using ::testing::_;
using testing::NiceMock;
using testing::Return;
using ::testing::SetArgPointee;
namespace pos
{
TEST(SmartLogMetaIo, SmartLogMetaIo_Constructor_Stack_Three_Arguments)
{
    uint32_t arrayIndex = 0;
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);
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
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);
    uint32_t expected = 0;
    uint32_t actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
}

TEST(SmartLogMetaIo, Init_CreateFileCreationSuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;

    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Create(_)).WillOnce(Return(0));
    uint32_t expected = 0;
    uint32_t actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
    delete metaFile;
}

TEST(SmartLogMetaIo, Init_CreateFileCreationUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Create(_)).WillOnce(Return(-1));
    uint32_t expected = 0;
    uint32_t actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
    delete metaFile;
}

TEST(SmartLogMetaIo, Init_FileLoad_OpenedFile_AsyncIOSuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, Close).WillOnce(Return(0));
    uint32_t expected = 0;
    uint32_t actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
    delete metaFile;
}

TEST(SmartLogMetaIo, Init_FileLoad_OpenedFile_AsyncIOSuccessful_Close_Unsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, Close).WillOnce(Return(2));
    uint32_t expected = 0;
    uint32_t actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
    delete metaFile;
}

TEST(SmartLogMetaIo, Init_FileLoad_OpenedFile_AsyncIOUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(-1));
    uint32_t expected = 0;
    uint32_t actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
    delete metaFile;
}

TEST(SmartLogMetaIo, Init_FileLoad_UnopenedFile_AsyncIOsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    uint32_t expected = 0;
    uint32_t actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
    delete metaFile;
}

TEST(SmartLogMetaIo, Init_FileLoad_UnopenedFile_AsyncIOUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, DoesFileExist).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(-1));
    uint32_t expected = 0;
    uint32_t actual = smartLogMetaIo.Init();
    ASSERT_EQ(expected, actual);
    delete metaFile;
}

TEST(SmartLogMetaIo, Dispose_SmartLogEnabledFalse)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(false));

    uint32_t arrayIndex = 0;
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;

    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);
    smartLogMetaIo.Dispose();
}

TEST(SmartLogMetaIo, Dispose_SmartLogEnabledTrue)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;
    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;

    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);
    smartLogMetaIo.Dispose();
}

TEST(SmartLogMetaIo, Dispose_FileOpenedAsyncIOSuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    smartLogMetaIo.Dispose();
    delete metaFile;
}

TEST(SmartLogMetaIo, Dispose_FileOpenedAsyncIOUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(true));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(-1));
    smartLogMetaIo.Dispose();
    delete metaFile;
}

TEST(SmartLogMetaIo, Dispose_FileUnopenedAsyncIOSuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    smartLogMetaIo.Dispose();
    delete metaFile;
}

TEST(SmartLogMetaIo, Dispose_FileUnopenedAsyncIOUnsuccessful)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);

    EXPECT_CALL(*metaFile, IsOpened).WillOnce(Return(false));
    EXPECT_CALL(*metaFile, Open).WillOnce(Return(0));
    EXPECT_CALL(*metaFile, AsyncIO).WillOnce(Return(0));
    smartLogMetaIo.Dispose();
    delete metaFile;
}

TEST(SmartLogMetaIo, Shutdown_Run)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);
    smartLogMetaIo.Shutdown();
}

TEST(SmartLogMetaIo, Flush_Run)
{
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, Init()).WillByDefault(Return());
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    uint32_t arrayIndex = 0;

    NiceMock<MockMetaFileIntf>* metaFile = new NiceMock<MockMetaFileIntf>;
    SmartLogMetaIo smartLogMetaIo(arrayIndex, &mockSmartLogMgr, metaFile);
    smartLogMetaIo.Flush();
}

} // namespace pos
