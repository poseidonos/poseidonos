#include "src/mapper/vsamap/vsamap_content.h"

#include <gtest/gtest.h>

#include "src/mapper/address/mapper_address_info.h"
#include "test/unit-tests/io/frontend_io/flush_command_manager_mock.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/map/map_mock.h"
#include "test/unit-tests/mapper/map/map_header_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(VSAMapContent, SetEntry_TestFailCase)
{
    NiceMock<MockMapperAddressInfo> info;
    NiceMock<MockFlushCmdManager>* fl = new NiceMock<MockFlushCmdManager>();
    NiceMock<MockMapHeader>* header = new NiceMock<MockMapHeader>(0);
    NiceMock<MockMap>* map = new NiceMock<MockMap>(0, 4032);
    EXPECT_CALL(info, GetArrayId).Times(1);
    EXPECT_CALL(*fl, IsInternalFlushEnabled).WillOnce(Return(true));
    VSAMapContent vsacon(0, &info, fl, map, header);

    VirtualBlkAddr vsa;
    vsa.offset = 0;
    vsa.stripeId = 0;
    vsacon.Init(5, 10, 4032);
    vsacon.GetEntriesPerPage();

    EXPECT_CALL(*map, GetMpageLock).Times(1);
    EXPECT_CALL(*map, GetMpage).WillOnce(Return(nullptr));
    EXPECT_CALL(*map, AllocateMpage).WillOnce(Return(nullptr));
    EXPECT_CALL(*map, ReleaseMpageLock).Times(1);
    int ret = vsacon.SetEntry(0, vsa);
    EXPECT_EQ(-EID(VSAMAP_SET_FAILURE), ret);

    delete fl;
}

TEST(VSAMapContent, SetEntry_Success)
{
    NiceMock<MockMapperAddressInfo> info;
    NiceMock<MockFlushCmdManager>* fl = new NiceMock<MockFlushCmdManager>();
    NiceMock<MockMapHeader>* header = new NiceMock<MockMapHeader>(0);
    NiceMock<MockMap>* map = new NiceMock<MockMap>(0, 4032);
    EXPECT_CALL(*fl, IsInternalFlushEnabled).WillOnce(Return(true));
    EXPECT_CALL(info, GetArrayId).Times(1);
    VSAMapContent vsacon(0, &info, fl, map, header);
    VirtualBlkAddr vsa;
    vsa.offset = 0;
    vsa.stripeId = 0;
    vsacon.Init(5, 10, 4032);
    char buf[4032];
    EXPECT_CALL(*map, GetMpage).WillOnce(Return(buf));
    EXPECT_CALL(*header, SetTouchedMpageBit).Times(1);
    EXPECT_CALL(*header, GetNumTouchedMpagesSet).WillOnce(Return(5));
    EXPECT_CALL(*header, GetNumTotalTouchedMpages).WillOnce(Return(10));
    EXPECT_CALL(*fl, UpdateVSANewEntries).Times(1);
    int ret = vsacon.SetEntry(0, vsa);
    EXPECT_EQ(0, ret);

    delete fl;
}

} // namespace pos
