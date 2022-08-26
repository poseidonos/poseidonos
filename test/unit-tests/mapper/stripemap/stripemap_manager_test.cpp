#include "src/mapper/stripemap/stripemap_manager.h"

#include <gtest/gtest.h>

#include "src/mapper/address/mapper_address_info.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/stripemap/stripemap_content_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(StripeMapManager, Init_TestFailCase0)
{
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    StripeMapManager smap(tp, con, &eventScheduler, &addrInfo);

    EXPECT_CALL(*con, OpenMapFile).WillOnce(Return(EID(NEED_TO_INITIAL_STORE)));
    EXPECT_CALL(*con, FlushHeader).WillOnce(Return(-1));
    int ret = smap.Init();
    EXPECT_EQ(-1, ret);
    delete tp;
}

TEST(StripeMapManager, Init_TestFailCase1)
{
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    NiceMock<MockEventScheduler> eventScheduler;
    StripeMapManager smap(nullptr, con, &eventScheduler, &addrInfo);

    EXPECT_CALL(*con, OpenMapFile).WillOnce(Return(-1));
    int ret = smap.Init();
    EXPECT_EQ(-1, ret);
}

TEST(StripeMapManager, Init_TestFailCase2)
{
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    NiceMock<MockEventScheduler> eventScheduler;
    StripeMapManager smap(nullptr, con, &eventScheduler, &addrInfo);

    EXPECT_CALL(*con, OpenMapFile).WillOnce(Return(0));
    EXPECT_CALL(*con, Load).WillOnce(Return(ERRID(MAP_LOAD_COMPLETED)));
    int ret = smap.Init();
    EXPECT_EQ(ERRID(MAP_LOAD_COMPLETED), ret);
}

TEST(StripeMapManager, FlushDirtyPageGiven_TestFailCase0)
{
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockTelemetryPublisher> tp;
    StripeMapManager smap(&tp, con, &eventScheduler, &addrInfo);

    MpageList d;
    EXPECT_CALL(addrInfo, GetArrayName).Times(1);
    EXPECT_CALL(*con, FlushDirtyPagesGiven).WillOnce(Return(0));
    int ret = smap.FlushDirtyPagesGiven(d, nullptr);
    EXPECT_EQ(0, ret);
    ret = smap.FlushDirtyPagesGiven(d, nullptr);
    EXPECT_LE(ERRID(MAP_FLUSH_IN_PROGRESS), ret);
}

TEST(StripeMapManager, FlushDirtyPageGiven_TestFailCase1)
{
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    NiceMock<MockTelemetryPublisher> tp;
    StripeMapManager smap(&tp, con, nullptr, &addrInfo);

    MpageList d;
    EXPECT_CALL(addrInfo, GetArrayName).Times(1);
    EXPECT_CALL(*con, FlushDirtyPagesGiven).WillOnce(Return(-1));
    int ret = smap.FlushDirtyPagesGiven(d, nullptr);
    EXPECT_EQ(-1, ret);
}

TEST(StripeMapManager, FlushTouchedPages_TestFailCase0)
{
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockTelemetryPublisher> tp;
    StripeMapManager smap(&tp, con, &eventScheduler, &addrInfo);

    MpageList d;
    EXPECT_CALL(addrInfo, GetArrayName).Times(1);
    EXPECT_CALL(*con, FlushTouchedPages).WillOnce(Return(0));
    int ret = smap.FlushTouchedPages(nullptr);
    EXPECT_EQ(0, ret);
    ret = smap.FlushTouchedPages(nullptr);
    EXPECT_LE(ERRID(MAP_FLUSH_IN_PROGRESS), ret);
}

TEST(StripeMapManager, FlushTouchedPages_TestFailCase1)
{
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockTelemetryPublisher> tp;
    StripeMapManager smap(&tp, con, &eventScheduler, &addrInfo);

    EXPECT_CALL(addrInfo, GetArrayName).Times(1);
    EXPECT_CALL(*con, FlushTouchedPages).WillOnce(Return(-1));
    int ret = smap.FlushTouchedPages(nullptr);
    EXPECT_EQ(-1, ret);
}

TEST(StripeMapManager, WaitWritePendingIoDone_TestSimpleCaller)
{
    MapperAddressInfo addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    StripeMapManager smap(nullptr, con, nullptr, &addrInfo);
    smap.WaitWritePendingIoDone();
}

TEST(StripeMapManager, GetStripeMapContent_TestSimpleCaller)
{
    MapperAddressInfo addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    NiceMock<MockStripeMapContent>* con2 = new NiceMock<MockStripeMapContent>();
    NiceMock<MockEventScheduler> eventScheduler;
    StripeMapManager smap(nullptr, con, &eventScheduler, &addrInfo);
    StripeMapContent* ret = smap.GetStripeMapContent();
    EXPECT_EQ(con, ret);
    smap.SetStripeMapContent(con2);
}

TEST(StripeMapManager, SetLSA_TestSimpleCaller)
{
    MapperAddressInfo addrInfo;
    NiceMock<MockStripeMapContent>* con = new NiceMock<MockStripeMapContent>();
    NiceMock<MockEventScheduler> eventScheduler;
    StripeMapManager smap(nullptr, con, &eventScheduler, &addrInfo);

    EXPECT_CALL(*con, SetEntry).WillOnce(Return(-1));
    int ret = smap.SetLSA(0, 0, IN_WRITE_BUFFER_AREA);
    EXPECT_EQ(-1, ret);
}

} // namespace pos
