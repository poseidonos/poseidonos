#include "src/mapper/stripemap/stripemap_manager.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(StripeMapManager, StripeMapManager_)
{
}

TEST(StripeMapManager, Init_)
{
}

TEST(StripeMapManager, StoreMap_)
{
}

TEST(StripeMapManager, FlushMap_)
{
}

TEST(StripeMapManager, Close_)
{
}

TEST(StripeMapManager, GetStripeMapContent_)
{
}

TEST(StripeMapManager, AllMapsAsyncFlushed_)
{
}

TEST(StripeMapManager, MapAsyncFlushDone_)
{
}

TEST(StripeMapManager, GetLSA_)
{
}

TEST(StripeMapManager, GetLSAandReferLsid_)
{
}

TEST(StripeMapManager, GetRandomLsid_)
{
}

TEST(StripeMapManager, SetLSA_)
{
}

TEST(StripeMapManager, IsInUserDataArea_)
{
}

TEST(StripeMapManager, IsInWriteBufferArea_)
{
}

TEST(StripeMapManager, GetDirtyStripeMapPages_)
{
}

TEST(StripeMapManager, AllocateUserDataStripeId_TestSimpleGetter)
{
    // given
    NiceMock<MockEventScheduler>* e = new NiceMock<MockEventScheduler>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    StripeMapManager stripeManager(e, addrInfo);
    // when
    stripeManager.AllocateUserDataStripeId(0);

    delete e;
    delete addrInfo;
}

} // namespace pos
