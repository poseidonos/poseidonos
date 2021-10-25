#include "src/mapper/vsamap/vsamap_content.h"

#include <gtest/gtest.h>

#include "src/mapper/address/mapper_address_info.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/io/frontend_io/flush_command_manager_mock.h"
#include "test/unit-tests/allocator/block_manager/block_manager_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
    TEST(VSAMapContent, SetEntry_TestFailCase)
    {
        MapperAddressInfo info;
        info.SetMPageSize(10);
        NiceMock<MockFlushCmdManager>* fl = new NiceMock<MockFlushCmdManager>();
        NiceMock<MockBlockManager>* bm = new NiceMock<MockBlockManager>();
        VSAMapContent vsacon(0, &info, bm, fl);

        VirtualBlkAddr vsa;
        vsa.offset = 0;
        vsa.stripeId = 0;
        int ret = vsacon.SetEntry(0, vsa);
        EXPECT_EQ(-EID(VSAMAP_SET_FAILURE), ret);
        
        delete fl;
        delete bm;
    }

    TEST(VSAMapContent, SetEntry_TestFailCase)
    {
        MapperAddressInfo info;
        info.SetMPageSize(10);
        NiceMock<MockFlushCmdManager>* fl = new NiceMock<MockFlushCmdManager>();
        NiceMock<MockBlockManager>* bm = new NiceMock<MockBlockManager>();
        EXPECT_CALL(*fl, IsInternalFlushEnabled).WillOnce(Return(true));
        VSAMapContent vsacon(0, &info, bm, fl);

        VirtualBlkAddr vsa;
        vsa.offset = 0;
        vsa.stripeId = 0;
        EXPECT_CALL(*fl, UpdateVSANewEntries).WillOnce(Return(true));
        int ret = vsacon.SetEntry(0, vsa);
        EXPECT_EQ(-EID(VSAMAP_SET_FAILURE), ret);
        
        delete fl;
        delete bm;
    }
} // namespace pos
