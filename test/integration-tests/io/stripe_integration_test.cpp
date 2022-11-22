#include <gtest/gtest.h>

#include "src/allocator/stripe_manager/stripe.h"
#include "src/allocator/stripe_manager/wbstripe_manager.h"
#include "src/mapper/reversemap/reverse_map.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/stripe_manager/stripe_load_status_mock.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/mapper/i_reversemap_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/resource_manager/memory_manager_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include "test/unit-tests/volume/i_volume_info_manager_mock.h"

using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;

namespace pos
{
class StripeIntegrationTest : public ::testing::Test
{
protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    WBStripeManager* wbstripeManager;
    AllocatorAddressInfo info;

    NiceMock<MockTelemetryPublisher> telemetryPublisher;
    NiceMock<MockIReverseMap> reverseMap;
    NiceMock<MockIVolumeInfoManager> volumeInfo;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockAllocatorCtx> allocatorCtx;
    NiceMock<MockStripeLoadStatus>* stripeLoadStatus;
    NiceMock<MockMemoryManager> memoryManager;
    NiceMock<MockEventScheduler> eventScheduler;

private:
    void _SetAllocatorAddressInfo(void);
};

void
StripeIntegrationTest::SetUp(void)
{
    _SetAllocatorAddressInfo();

    ON_CALL(reverseMap, AllocReverseMapPack).WillByDefault([&](StripeId vsid, StripeId wblsid) {
        return new ReverseMapPack(vsid, wblsid, 4032, 1);
    });

    int numVolumes = 256;
    stripeLoadStatus = new NiceMock<MockStripeLoadStatus>();
    wbstripeManager = new WBStripeManager(&telemetryPublisher, numVolumes,
        &reverseMap, &volumeInfo, &stripeMap, &allocatorCtx, &info, stripeLoadStatus,
        "TestArray", 0, &memoryManager, &eventScheduler);

    wbstripeManager->Init();
}

void
StripeIntegrationTest::TearDown(void)
{
    wbstripeManager->Dispose();
    delete wbstripeManager;
}

void
StripeIntegrationTest::_SetAllocatorAddressInfo(void)
{
    info.SetblksPerStripe(32);
    info.SetnumWbStripes(1024);
    info.SetchunksPerStripe(1);
    info.SetstripesPerSegment(64);
    info.SetnumUserAreaSegments(8);
    info.SetnumUserAreaStripes(64 * 8);
    info.SetblksPerSegment(8 * 64 * 32);
}

TEST_F(StripeIntegrationTest, TestUserStripeAllocation)
{
    StripeId vsid = 10, wbLsid = 3, userLsid = 10;
    int volumeId = 2;

    StripeSmartPtr stripe = StripeSmartPtr(new Stripe(&reverseMap, info.GetblksPerStripe()));
    stripe->Assign(vsid, wbLsid, userLsid, volumeId);
    wbstripeManager->AssignStripe(stripe);

    EXPECT_EQ(stripe->GetVsid(), vsid);
    EXPECT_EQ(stripe->GetWbLsid(), wbLsid);
    EXPECT_EQ(stripe->GetVolumeId(), volumeId);
}

TEST_F(StripeIntegrationTest, TestGcStripeAllocation)
{
    StripeId vsid = 23, wbLsid = UNMAP_STRIPE, userLsid = 23;
    int volumeId = 3;

    StripeSmartPtr stripe = StripeSmartPtr(new Stripe(&reverseMap, info.GetblksPerStripe()));
    stripe->Assign(vsid, wbLsid, userLsid, volumeId);
    EXPECT_EQ(stripe->GetVsid(), vsid);
    EXPECT_EQ(stripe->GetWbLsid(), wbLsid);
    EXPECT_EQ(stripe->GetVolumeId(), volumeId);
}

TEST_F(StripeIntegrationTest, TestUserStripeAllocationToFlush)
{
    StripeId vsid = 10, wbLsid = 3, userLsid = 10;
    int volumeId = 2;

    // Stripe allocation
    StripeSmartPtr stripe = StripeSmartPtr(new Stripe(&reverseMap, info.GetblksPerStripe()));
    stripe->Assign(vsid, wbLsid, userLsid, volumeId);
    wbstripeManager->AssignStripe(stripe);

    // Update reversemap
    std::vector<BlkAddr> rbas;
    for (uint32_t offset = 0; offset < info.GetblksPerStripe(); offset++)
    {
        BlkAddr rba = std::rand() % (info.GetblksPerSegment() * info.GetnumUserAreaSegments());
        stripe->UpdateReverseMapEntry(offset, rba, volumeId);
        rbas.push_back(rba);
    }

    // Verify reversemap
    for (uint32_t offset = 0; offset < info.GetblksPerStripe(); offset++)
    {
        BlkAddr readRba;
        uint32_t readVolumeId;
        std::tie(readRba, readVolumeId) = stripe->GetReverseMapEntry(offset);
        EXPECT_EQ(rbas[offset], readRba);
        EXPECT_EQ(volumeId, readVolumeId);
    }

    // Flush
    ReverseMapPack* req;
    EXPECT_CALL(reverseMap, Flush).WillOnce(DoAll(SaveArg<0>(&req), Return(0)));

    EventSmartPtr callback(new NiceMock<MockEvent>);
    int flushResult = stripe->Flush(callback);
    EXPECT_EQ(flushResult, 0);

    for (uint32_t offset = 0; offset < info.GetblksPerStripe(); offset++)
    {
        BlkAddr readRba;
        uint32_t readVolumeId;
        std::tie(readRba, readVolumeId) = req->GetReverseMapEntry(offset);
        EXPECT_EQ(rbas[offset], readRba);
        EXPECT_EQ(volumeId, readVolumeId);
    }
}

} // namespace pos
