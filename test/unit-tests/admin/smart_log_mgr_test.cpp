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
    MockConfigManager mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(1));
    SmartLogMgr smartLogMgr(&mockConfigManager);
    smartLogMgr.Init();
}

TEST(SmartLogMgr, Init_GetValue_Failure)
{
    MockConfigManager mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(0));
    SmartLogMgr smartLogMgr(&mockConfigManager);
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
    NiceMock<MockConfigManager>* mockConfigManager = CreateMockConfigManager(true);
    SmartLogMgr smartLogMgr(mockConfigManager);
    smartLogMgr.Init();
    bool expected = true;
    smartLogMgr.GetSmartLogEnabled();
    delete mockConfigManager;
}

TEST(SmartLogMgr, GetLogPages_Run)
{
    SmartLogMgr smartLogMgr;
    smartLogMgr.Init();
    uint32_t arrayIndex = 0;
    smartLogMgr.GetLogPages(arrayIndex);
}

TEST(SmartLogMgr, IncreaseReadCmds_Setter_Getter_Test)
{
    NiceMock<MockConfigManager>* mockConfigManager = CreateMockConfigManager(true);
    SmartLogMgr smartLogMgr(mockConfigManager);
    smartLogMgr.Init();
    uint32_t volId = 0;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseReadCmds(volId, arrayId);
    uint64_t expectedReadCmds = 1;
    uint64_t actualReadCmds;
    actualReadCmds = smartLogMgr.GetReadCmds(volId, arrayId);
    // commenting the assert as main code needs to change, for the assert to pass
    ASSERT_EQ(expectedReadCmds, actualReadCmds);
    delete mockConfigManager;
}

TEST(SmartLogMgr, WriteCmds_Getter_Setter_Test)
{
    NiceMock<MockConfigManager>* mockConfigManager = CreateMockConfigManager(true);
    SmartLogMgr smartLogMgr(mockConfigManager);
    smartLogMgr.Init();
    uint32_t volId = 2;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseWriteCmds(volId, arrayId);
    uint64_t expectedWriteCmds = 1;
    uint64_t actualWriteCmds;
    actualWriteCmds = smartLogMgr.GetWriteCmds(volId, arrayId);
    ASSERT_EQ(expectedWriteCmds, actualWriteCmds);
    delete mockConfigManager;
}

TEST(SmartLogMgr, IncreaseReadBytes_Setter_Getter_Test)
{
    NiceMock<MockConfigManager>* mockConfigManager = CreateMockConfigManager(true);
    SmartLogMgr smartLogMgr(mockConfigManager);
    smartLogMgr.Init();
    uint32_t volId = 4;
    uint64_t blockCount = 1;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseReadBytes(blockCount, volId, arrayId);
    uint64_t expectedReadBytes = blockCount * pos::BLOCK_SIZE;
    uint64_t actualReadBytes;
    actualReadBytes = smartLogMgr.GetReadBytes(volId, arrayId);
    ASSERT_EQ(expectedReadBytes, actualReadBytes);
    delete mockConfigManager;
}

TEST(SmartLogMgr, IncreaseWriteBytes_Getter_Setter_test)
{
    NiceMock<MockConfigManager>* mockConfigManager = CreateMockConfigManager(true);
    SmartLogMgr smartLogMgr(mockConfigManager);
    smartLogMgr.Init();
    uint32_t volId = 5;
    uint64_t blockCount = 1;
    uint32_t arrayId = 0;
    smartLogMgr.IncreaseWriteBytes(blockCount, volId, arrayId);
    uint64_t expectedWriteBytes = blockCount * pos::BLOCK_SIZE;
    uint64_t actualWriteBytes;
    actualWriteBytes = smartLogMgr.GetWriteBytes(volId, arrayId);
    ASSERT_EQ(expectedWriteBytes, actualWriteBytes);
    delete mockConfigManager;
}

} // namespace pos
