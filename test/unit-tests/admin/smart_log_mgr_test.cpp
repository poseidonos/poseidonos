#include "src/admin/smart_log_mgr.h"

#include <gtest/gtest.h>

#include "test/unit-tests/master_context/config_manager_mock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;
namespace pos
{
ACTION_P(SetArg2ToBoolAndReturn0, boolValue)
{
    *static_cast<bool*>(arg2) = boolValue;
    return 0;
}

TEST(SmartLogMgr, SmartLogMgr_Constructor_One_Stack)
{
    SmartLogMgr smartLogMgr();
}

TEST(SmartLogMgr, SmartLogMgr_Constructor_One_Heap)
{
    SmartLogMgr* smartLogMgr = new SmartLogMgr();
    delete smartLogMgr;
}

TEST(SmartLogMgr, Init_GetValue_Success)
{
    SmartLogMgr smartLogMgr;
    MockConfigManager mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(1));
    smartLogMgr.Init();
}

TEST(SmartLogMgr, Init_GetValue_Failure)
{
    MockConfigManager mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(0));
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
}

NiceMock<MockConfigManager>*
CreateMockConfigManager(bool isSmartLogEnabled)
{
    NiceMock<MockConfigManager>* configManager = new NiceMock<MockConfigManager>;

    ON_CALL(*configManager, GetValue("admin", "smart_log_page", _, _)).WillByDefault(SetArg2ToBoolAndReturn0(isSmartLogEnabled));
    return configManager;
}

TEST(SmartLogMgr, GetSmartLogEnabled_Config)
{
    MockConfigManager mockConfigManager;

    NiceMock<MockConfigManager>* configManager = CreateMockConfigManager(true);
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    bool expected = true;
    smartLogMgr.GetSmartLogEnabled();
    delete configManager;
}

TEST(SmartLogMgr, GetLogPages_Run)
{
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    uint32_t arrayIndex = 0;
    smartLogMgr.GetLogPages(arrayIndex);
}

TEST(SmartLogMgr, IncreaseReadCmds_Setter_Test_Read_Cmds)
{
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    uint32_t volId = 0;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseReadCmds(volId, arrayId);
    uint64_t expectedReadCmds = 1;
    uint64_t actualReadCmds;
    actualReadCmds = smartLogMgr.GetReadCmds(volId, arrayId);
    ASSERT_EQ(expectedReadCmds, actualReadCmds);
}

TEST(SmartLogMgr, WriteCmds_Getter_Setter_Test_Write_Cmds)
{
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    uint32_t volId = 2;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseWriteCmds(volId, arrayId);
    uint64_t expectedWriteCmds = 1;
    uint64_t actualWriteCmds;
    actualWriteCmds = smartLogMgr.GetWriteCmds(volId, arrayId);
    ASSERT_EQ(expectedWriteCmds, actualWriteCmds);
}

TEST(SmartLogMgr, GetReadCmds_Getter_Test_Read_Cmds)
{
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    uint32_t volId = 3;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseWriteCmds(volId, arrayId);
    uint64_t expectedWriteCmds = 1;
    uint64_t actualWriteCmds;
    actualWriteCmds = smartLogMgr.GetWriteCmds(volId, arrayId);
    ASSERT_EQ(expectedWriteCmds, actualWriteCmds);
}

TEST(SmartLogMgr, IncreaseReadBytes_Setter_Test_Read_Bytes)
{
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    uint32_t volId = 4;
    uint64_t blockCount = 1;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseReadBytes(blockCount, volId, arrayId);
    uint64_t expectedReadBytes = blockCount * pos::BLOCK_SIZE;
    uint64_t actualReadBytes;
    actualReadBytes = smartLogMgr.GetReadBytes(volId, arrayId);
    ASSERT_EQ(expectedReadBytes, actualReadBytes);
}

TEST(SmartLogMgr, IncreaseWriteBytes_Setter_Test_Write_Bytes)
{
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    uint32_t volId = 5;
    uint64_t blockCount = 1;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseWriteBytes(blockCount, volId, arrayId);
    uint64_t expectedWriteBytes = blockCount * pos::BLOCK_SIZE;
    uint64_t actualWriteBytes;
    actualWriteBytes = smartLogMgr.GetWriteBytes(volId, arrayId);
    ASSERT_EQ(expectedWriteBytes, actualWriteBytes);
}

TEST(SmartLogMgr, GetWriteBytes_Getter_Test_Write_Bytes)
{
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    uint32_t volId = 6;
    uint64_t blockCount = 1;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseWriteBytes(blockCount, volId, arrayId);
    uint64_t expectedWriteBytes = blockCount * pos::BLOCK_SIZE;
    uint64_t actualWriteBytes;
    actualWriteBytes = smartLogMgr.GetWriteBytes(volId, arrayId);
    ASSERT_EQ(expectedWriteBytes, actualWriteBytes);
}

TEST(SmartLogMgr, GetReadBytes_Getter_Test_Read_Bytes)
{
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    uint32_t volId = 7;
    uint64_t blockCount = 1;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseReadBytes(blockCount, volId, arrayId);
    uint64_t expectedReadBytes = blockCount * pos::BLOCK_SIZE;
    uint64_t actualReadBytes;
    actualReadBytes = smartLogMgr.GetReadBytes(volId, arrayId);
    ASSERT_EQ(expectedReadBytes, actualReadBytes);
}

} // namespace pos
