#include <gtest/gtest.h>

#include "src/array_models/dto/partition_logical_size.h"
#include "src/gc/victim_stripe.h"
#include "src/include/partition_type.h"
#include "src/mapper/reversemap/reverse_map.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/mapper/i_reversemap_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "test/unit-tests/volume/i_volume_io_manager_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class VictimStripeIntegrationTest : public ::testing::Test
{
protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    VictimStripe* victimStripe;

    PartitionLogicalSize logicalSize;

    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIReverseMap> reverseMap;
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIVolumeIoManager> volumeManager;

private:
    void _SetLogicalSize(void);
};

void
VictimStripeIntegrationTest::SetUp(void)
{
    _SetLogicalSize();

    ON_CALL(reverseMap, AllocReverseMapPack).WillByDefault([&](StripeId vsid, StripeId wblsid) {
        return new ReverseMapPack(vsid, wblsid, 4032, 1);
    });

    victimStripe = new VictimStripe(&arrayInfo, &reverseMap, &vsaMap, &stripeMap, &volumeManager);
}

void
VictimStripeIntegrationTest::TearDown(void)
{
    delete victimStripe;
}

void
VictimStripeIntegrationTest::_SetLogicalSize(void)
{
    logicalSize.blksPerChunk = 128;
    logicalSize.blksPerStripe = 128 * 4;
    logicalSize.chunksPerStripe = 4;
    logicalSize.stripesPerSegment = 1024;
    logicalSize.totalStripes = 1024 * 10;
    logicalSize.totalSegments = 10;

    ON_CALL(arrayInfo, GetSizeInfo(PartitionType::USER_DATA)).WillByDefault(Return(&logicalSize));
}

// Test if victim stripe request load reversemap properly
TEST_F(VictimStripeIntegrationTest, TestReverseMapLoad)
{
    StripeId lsid = 102;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));

    EXPECT_CALL(reverseMap, Load).WillOnce([&](ReverseMapPack* revMapPack, EventSmartPtr cb) {
        EXPECT_EQ(revMapPack->GetVsid(), lsid);
        return 0;
    });

    victimStripe->Load(lsid, callback);
}

} // namespace pos