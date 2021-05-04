#include "src/journal_manager/replay/active_wb_stripe_replayer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <vector>

#include "test/unit-tests/allocator/i_context_replayer_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"

using testing::NiceMock;
using ::testing::Return;

namespace pos
{
const uint32_t numStripesPerSegment = 64;
const int numUserSegments = 64;
const int numBlksPerChunk = 32;
const int numChunksPerStripe = 4;

const int numWbStripes = 64;
const uint32_t numUserStripes = numStripesPerSegment * numUserSegments;
const uint64_t numBlksPerStripe = numBlksPerChunk * numChunksPerStripe;
const int defaultTestVol = 1;

StripeInfo
GetActiveStripe(int volumeId, int wbufIndex)
{
    StripeId vsid = std::rand() % numUserStripes;
    StripeId wbLsid = std::rand() % numWbStripes;
    StripeId userLsid = vsid;
    BlkOffset lastOffset = std::rand() % (numBlksPerStripe - 1);

    StripeInfo activeStripe(volumeId, vsid, wbLsid, userLsid, lastOffset, wbufIndex);

    return activeStripe;
}

StripeInfo
GetFlushedActiveStripe(int volumeId, int wbufIndex)
{
    StripeInfo activeStripe = GetActiveStripe(volumeId, wbufIndex);
    activeStripe.ResetOffset();

    return activeStripe;
}

VirtualBlkAddr
GetTail(StripeInfo stripe)
{
    VirtualBlkAddr tail = {
        .stripeId = stripe.GetVsid(),
        .offset = stripe.GetLastOffset() + 1};

    return tail;
}

TEST(ActiveWBStripeReplayer, Replay_SingleActiveStripe)
{
    // Given
    NiceMock<MockIContextReplayer>* contextReplayer = new NiceMock<MockIContextReplayer>;
    NiceMock<MockIWBStripeAllocator>* wbStripeAllocator = new NiceMock<MockIWBStripeAllocator>;
    EXPECT_CALL(*contextReplayer, GetAllActiveStripeTail()).WillOnce(Return(std::vector<VirtualBlkAddr>(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA)));
    PendingStripeList pendingStripes;

    ActiveWBStripeReplayer* wbStripeReplayer = new ActiveWBStripeReplayer(contextReplayer, wbStripeAllocator, pendingStripes);

    // When : Find a single active stripe that is not full
    StripeInfo activeStripe = GetActiveStripe(defaultTestVol, defaultTestVol);

    wbStripeReplayer->Update(activeStripe);

    // Then : Will restore the active stipre tail to this stripe
    VirtualBlkAddr tail = GetTail(activeStripe);
    EXPECT_CALL(*(wbStripeAllocator),
        RestoreActiveStripeTail(activeStripe.GetWbIndex(), tail, activeStripe.GetWbLsid()))
        .Times(1);

    wbStripeReplayer->Replay();

    delete contextReplayer;
    delete wbStripeAllocator;
}

TEST(ActiveWBStripeReplayer, Replay_SingleFullActiveStripe)
{
    // Given
    NiceMock<MockIContextReplayer>* contextReplayer = new NiceMock<MockIContextReplayer>;
    NiceMock<MockIWBStripeAllocator>* wbStripeAllocator = new NiceMock<MockIWBStripeAllocator>;
    EXPECT_CALL(*contextReplayer, GetAllActiveStripeTail()).WillOnce(Return(std::vector<VirtualBlkAddr>(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA)));
    PendingStripeList pendingStripes;

    ActiveWBStripeReplayer* wbStripeReplayer = new ActiveWBStripeReplayer(contextReplayer, wbStripeAllocator, pendingStripes);

    // When : Find a single active stripe that saturated
    StripeInfo fullActiveStripe = GetFlushedActiveStripe(defaultTestVol, defaultTestVol);

    wbStripeReplayer->Update(fullActiveStripe);

    // Then : Will reset the active stripe tail to unmap_vsa
    EXPECT_CALL(*(contextReplayer),
        ResetActiveStripeTail(fullActiveStripe.GetWbIndex()))
        .Times(1);

    wbStripeReplayer->Replay();

    delete contextReplayer;
    delete wbStripeAllocator;
}

TEST(ActiveWBStripeReplayer, Replay_SingleVolumeMultipleStripe)
{
    // Given
    NiceMock<MockIContextReplayer>* contextReplayer = new NiceMock<MockIContextReplayer>;
    NiceMock<MockIWBStripeAllocator>* wbStripeAllocator = new NiceMock<MockIWBStripeAllocator>;
    EXPECT_CALL(*contextReplayer, GetAllActiveStripeTail()).WillOnce(Return(std::vector<VirtualBlkAddr>(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA)));
    PendingStripeList pendingStripes;

    ActiveWBStripeReplayer* wbStripeReplayer = new ActiveWBStripeReplayer(contextReplayer, wbStripeAllocator, pendingStripes);

    // When : Find several stripes on single volume
    for (int stripe = 0; stripe < 5; stripe++)
    {
        StripeInfo fullStripe = GetFlushedActiveStripe(defaultTestVol, defaultTestVol);

        wbStripeReplayer->Update(fullStripe);
    }

    StripeInfo lastActiveStripe = GetActiveStripe(defaultTestVol, defaultTestVol);
    wbStripeReplayer->Update(lastActiveStripe);

    // Then : Will restore the active stripe tail to the lastActiveStripe
    VirtualBlkAddr tail = GetTail(lastActiveStripe);
    EXPECT_CALL(*(wbStripeAllocator),
        RestoreActiveStripeTail(lastActiveStripe.GetWbIndex(), tail, lastActiveStripe.GetWbLsid()))
        .Times(1);

    wbStripeReplayer->Replay();

    delete contextReplayer;
    delete wbStripeAllocator;
}

TEST(ActiveWBStripeReplayer, Replay_MultiVolumeMultipleStripe)
{
    // Given
    NiceMock<MockIContextReplayer>* contextReplayer = new NiceMock<MockIContextReplayer>;
    NiceMock<MockIWBStripeAllocator>* wbStripeAllocator = new NiceMock<MockIWBStripeAllocator>;
    EXPECT_CALL(*contextReplayer, GetAllActiveStripeTail()).WillOnce(Return(std::vector<VirtualBlkAddr>(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA)));
    PendingStripeList pendingStripes;

    ActiveWBStripeReplayer* wbStripeReplayer = new ActiveWBStripeReplayer(contextReplayer, wbStripeAllocator, pendingStripes);

    // When : Find several stripes on multiple volumes
    std::vector<StripeInfo> stripes;
    std::vector<StripeInfo> lastStripesPerVolume;
    for (int volumeId = 1; volumeId < 4; volumeId++)
    {
        for (int stripe = 0; stripe < 5; stripe++)
        {
            StripeInfo fullStripe = GetFlushedActiveStripe(volumeId, volumeId);
            stripes.push_back(fullStripe);
        }

        StripeInfo lastActiveStripe = GetActiveStripe(volumeId, volumeId);
        stripes.push_back(lastActiveStripe);
        lastStripesPerVolume.push_back(lastActiveStripe);
    }

    auto rng = std::default_random_engine{};
    std::shuffle(stripes.begin(), stripes.end(), rng);

    for (auto it = stripes.begin(); it != stripes.end(); ++it)
    {
        wbStripeReplayer->Update(*it);
    }

    // Then : Will be restored active stripe tail to the last stripe updated for each voulme
    for (auto lastActiveStripe : lastStripesPerVolume)
    {
        VirtualBlkAddr tail = GetTail(lastActiveStripe);
        EXPECT_CALL(*(wbStripeAllocator),
            RestoreActiveStripeTail(lastActiveStripe.GetWbIndex(), tail, lastActiveStripe.GetWbLsid()))
            .Times(1);
    }
    wbStripeReplayer->Replay();

    delete contextReplayer;
    delete wbStripeAllocator;
}

TEST(ActiveWBStripeReplayer, Replay_SingleVolumeMultipleActiveStripe)
{
    // Given
    NiceMock<MockIContextReplayer>* contextReplayer = new NiceMock<MockIContextReplayer>;
    NiceMock<MockIWBStripeAllocator>* wbStripeAllocator = new NiceMock<MockIWBStripeAllocator>;
    EXPECT_CALL(*contextReplayer, GetAllActiveStripeTail()).WillOnce(Return(std::vector<VirtualBlkAddr>(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA)));
    PendingStripeList pendingStripes;

    ActiveWBStripeReplayer* wbStripeReplayer = new ActiveWBStripeReplayer(contextReplayer, wbStripeAllocator, pendingStripes);

    // When : Find several unflushed stripes on a single volume
    std::vector<StripeInfo> orphanStripes;
    for (int stripe = 0; stripe < 5; stripe++)
    {
        StripeInfo orphanStripe = GetActiveStripe(defaultTestVol, defaultTestVol);

        wbStripeReplayer->Update(orphanStripe);
        orphanStripes.push_back(orphanStripe);

        StripeInfo fullStripe = GetFlushedActiveStripe(defaultTestVol, defaultTestVol);
        wbStripeReplayer->Update(fullStripe);
    }

    StripeInfo lastStripe = GetActiveStripe(defaultTestVol, defaultTestVol);
    wbStripeReplayer->Update(lastStripe);

    // Then
    EXPECT_CALL(*(wbStripeAllocator),
        RestoreActiveStripeTail(lastStripe.GetWbIndex(), GetTail(lastStripe), lastStripe.GetWbLsid()))
        .Times(1);

    wbStripeReplayer->Replay();

    EXPECT_TRUE(orphanStripes.size() == pendingStripes.size());

    std::sort(orphanStripes.begin(), orphanStripes.end(), [](StripeInfo a, StripeInfo b) {
        return (a.GetWbLsid() < b.GetWbLsid());
    });

    std::sort(pendingStripes.begin(), pendingStripes.end(), [](PendingStripe* a, PendingStripe* b) {
        return (a->wbLsid < b->wbLsid);
    });

    for (uint32_t index = 0; index < orphanStripes.size(); index++)
    {
        EXPECT_TRUE(orphanStripes[index].GetWbLsid() == pendingStripes[index]->wbLsid);
        EXPECT_TRUE(GetTail(orphanStripes[index]) == pendingStripes[index]->tailVsa);
    }

    delete contextReplayer;
    delete wbStripeAllocator;
}

TEST(ActiveWBStripeReplayer, Replay_MultiVolumeMultipleActiveStripe)
{
    // Given
    NiceMock<MockIContextReplayer>* contextReplayer = new NiceMock<MockIContextReplayer>;
    NiceMock<MockIWBStripeAllocator>* wbStripeAllocator = new NiceMock<MockIWBStripeAllocator>;
    EXPECT_CALL(*contextReplayer, GetAllActiveStripeTail()).WillOnce(Return(std::vector<VirtualBlkAddr>(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA)));
    PendingStripeList pendingStripes;

    ActiveWBStripeReplayer* wbStripeReplayer = new ActiveWBStripeReplayer(contextReplayer, wbStripeAllocator, pendingStripes);

    // When : Find several unflushed stripes on a single volume
    std::vector<StripeInfo> orphanStripes;
    for (int volumeId = 0; volumeId < 5; volumeId++)
    {
        for (int stripe = 0; stripe < 5; stripe++)
        {
            StripeInfo orphanStripe = GetActiveStripe(volumeId, volumeId);
            wbStripeReplayer->Update(orphanStripe);
            orphanStripes.push_back(orphanStripe);

            StripeInfo fullStripe = GetFlushedActiveStripe(volumeId, volumeId);
            wbStripeReplayer->Update(fullStripe);
        }

        StripeInfo lastStripe = GetActiveStripe(volumeId, volumeId);
        wbStripeReplayer->Update(lastStripe);

        EXPECT_CALL(*(wbStripeAllocator),
            RestoreActiveStripeTail(lastStripe.GetWbIndex(), GetTail(lastStripe), lastStripe.GetWbLsid()))
            .Times(1);
    }

    // Then
    wbStripeReplayer->Replay();

    EXPECT_TRUE(orphanStripes.size() == pendingStripes.size());

    std::sort(orphanStripes.begin(), orphanStripes.end(), [](StripeInfo a, StripeInfo b) {
        return (a.GetWbLsid() < b.GetWbLsid());
    });

    std::sort(pendingStripes.begin(), pendingStripes.end(), [](PendingStripe* a, PendingStripe* b) {
        return (a->wbLsid < b->wbLsid);
    });

    for (uint32_t index = 0; index < orphanStripes.size(); index++)
    {
        EXPECT_TRUE(orphanStripes[index].GetWbLsid() == pendingStripes[index]->wbLsid);
        EXPECT_TRUE(GetTail(orphanStripes[index]) == pendingStripes[index]->tailVsa);
    }

    delete contextReplayer;
    delete wbStripeAllocator;
}
} // namespace pos
