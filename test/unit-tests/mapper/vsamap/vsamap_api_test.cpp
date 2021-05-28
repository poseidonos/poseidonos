#include "src/mapper/include/mapper_const.h"
#include "src/mapper/vsamap/vsamap_api.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/vsamap/vsamap_content_mock.h"
#include "test/unit-tests/mapper/vsamap/i_vsamap_internal_mock.h"

#include <gtest/gtest.h>

using namespace ::testing;

namespace pos
{
TEST(VSAMapAPI, VSAMapAPI_)
{
    // function touched
}

TEST(VSAMapAPI, EnableVsaMapAccess_)
{
    // function touched
}

TEST(VSAMapAPI, DisableVsaMapAccess_Default)
{
    // Given
    const int VOLID = 0;
    VSAMapAPI sut(nullptr, nullptr);

    // When
    sut.DisableVsaMapAccess(VOLID);

    // Then
}

TEST(VSAMapAPI, GetVSAMapContent_)
{
    // function touched
}

TEST(VSAMapAPI, GetVSAs_VolumeIsNotAccessible)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    const uint32_t NUM_BLKS = 3;
    int reti = 0;
    
    VSAMapAPI sut(nullptr, nullptr);

    // When
    VsaArray vsaArray;
    reti = sut.GetVSAs(VOLID, START_RBA, NUM_BLKS, vsaArray);

    // Then
    EXPECT_EQ(reti, -EID(VSAMAP_NOT_ACCESSIBLE));
    EXPECT_EQ(vsaArray[NUM_BLKS - 1], UNMAP_VSA);
}

TEST(VSAMapAPI, GetVSAs_VolumeIsAccessible)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    const uint32_t NUM_BLKS = 3;
    int reti = 0;

    MockVSAMapContent* mockVSAMapContent = new MockVSAMapContent;
    VirtualBlkAddr vsa {.stripeId = 1, .offset = 2};
    EXPECT_CALL(*mockVSAMapContent, GetEntry).WillRepeatedly(Return(vsa));

    VSAMapAPI sut(nullptr, nullptr);
    sut.EnableVsaMapAccess(VOLID);
    VSAMapContent*& vsaMapContent = sut.GetVSAMapContent(VOLID);   // Get Reference
    vsaMapContent = mockVSAMapContent; // Replace with Mock

    // When
    VsaArray vsaArray;
    reti = sut.GetVSAs(VOLID, START_RBA, NUM_BLKS, vsaArray);

    // Then
    EXPECT_EQ(reti, 0);
    EXPECT_EQ(vsaArray[NUM_BLKS - 1], vsa);
    vsaMapContent = nullptr;    // Replace nullptr again, due to vsaMapAPISUT Dtor crash
    delete mockVSAMapContent;
}

TEST(VSAMapAPI, SetVSAs_VolumeIsNotAccessible)
{
    // Given
    int reti = 0;
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    VirtualBlks vsas;

    VSAMapAPI sut(nullptr, nullptr);

    // When
    reti = sut.SetVSAs(VOLID, START_RBA, vsas);

    // Then    
    EXPECT_EQ(reti, -EID(VSAMAP_NOT_ACCESSIBLE));
}

TEST(VSAMapAPI, SetVSAs_VolumeIsAccessible)
{
    // Given
    int reti = 0;
    const int VOLID = 0;    
    const BlkAddr START_RBA = 0;
    const uint32_t NUM_BLKS = 3;
    VirtualBlkAddr vsa {.stripeId = 1, .offset = 2};    
    VirtualBlks vsas {.startVsa = vsa, .numBlks = NUM_BLKS};

    MockVSAMapContent* mockVSAMapContent = new MockVSAMapContent;
    EXPECT_CALL(*mockVSAMapContent, SetEntry).WillRepeatedly(Return(0));

    VSAMapAPI sut(nullptr, nullptr);
    sut.EnableVsaMapAccess(VOLID);
    VSAMapContent*& vsaMapContent = sut.GetVSAMapContent(VOLID);   // Get Reference
    vsaMapContent = mockVSAMapContent; // Replace with Mock

    // When
    reti = sut.SetVSAs(VOLID, START_RBA, vsas);

    // Then
    EXPECT_EQ(reti, 0);
    vsaMapContent = nullptr;    // Replace nullptr again, due to vsaMapAPISUT Dtor crash
    delete mockVSAMapContent;
}

TEST(VSAMapAPI, GetVSAInternal_Refused)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    int caller = CALLER_NOT_EVENT;

    MockIVSAMapInternal* mockIVSAMapInternal = new MockIVSAMapInternal;
    EXPECT_CALL(*mockIVSAMapInternal, EnableInternalAccess).WillRepeatedly(Return(-1));

    VSAMapAPI sut(mockIVSAMapInternal, nullptr);

    // When
    VirtualBlkAddr vsa = sut.GetVSAInternal(VOLID, START_RBA, caller);

    // Then
    EXPECT_EQ(vsa, UNMAP_VSA);
    delete mockIVSAMapInternal;
}

TEST(VSAMapAPI, GetVSAInternal_NotEvent)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    int caller = CALLER_NOT_EVENT;

    MockIVSAMapInternal* mockIVSAMapInternal = new MockIVSAMapInternal;
    EXPECT_CALL(*mockIVSAMapInternal, EnableInternalAccess).WillRepeatedly(Return(0));

    MockVSAMapContent* mockVSAMapContent = new MockVSAMapContent;
    VirtualBlkAddr vsa {.stripeId = 1, .offset = 2};
    EXPECT_CALL(*mockVSAMapContent, GetEntry).WillRepeatedly(Return(vsa));

    VSAMapAPI sut(mockIVSAMapInternal, nullptr);
    VSAMapContent*& vsaMapContent = sut.GetVSAMapContent(VOLID);   // Get Reference
    vsaMapContent = mockVSAMapContent; // Replace with Mock

    // When
    VirtualBlkAddr rvsa = sut.GetVSAInternal(VOLID, START_RBA, caller);

    // Then
    EXPECT_EQ(rvsa, vsa);
    vsaMapContent = nullptr;    // Replace nullptr again, due to vsaMapAPISUT Dtor crash
    delete mockIVSAMapInternal;
    delete mockVSAMapContent;
}

TEST(VSAMapAPI, GetVSAInternal_EventLoadDone)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    int caller = CALLER_EVENT;
    std::atomic<int> loaded;
    std::atomic<int>& ref = loaded;
    
    MockIVSAMapInternal* mockIVSAMapInternal = new MockIVSAMapInternal;
    EXPECT_CALL(*mockIVSAMapInternal, EnableInternalAccess).WillRepeatedly(Return(0));
    loaded = LOAD_DONE;
    EXPECT_CALL(*mockIVSAMapInternal, GetLoadDoneFlag).WillRepeatedly(ReturnRef(ref));

    MockVSAMapContent* mockVSAMapContent = new MockVSAMapContent;
    VirtualBlkAddr vsa {.stripeId = 1, .offset = 2};
    EXPECT_CALL(*mockVSAMapContent, GetEntry).WillRepeatedly(Return(vsa));

    VSAMapAPI sut(mockIVSAMapInternal, nullptr);
    VSAMapContent*& vsaMapContent = sut.GetVSAMapContent(VOLID);   // Get Reference
    vsaMapContent = mockVSAMapContent; // Replace with Mock

    // When
    VirtualBlkAddr rvsa = sut.GetVSAInternal(VOLID, START_RBA, caller);

    // Then
    EXPECT_EQ(rvsa, vsa);
    vsaMapContent = nullptr;    // Replace nullptr again, due to vsaMapAPISUT Dtor crash
    delete mockIVSAMapInternal;
    delete mockVSAMapContent;
}

TEST(VSAMapAPI, GetVSAInternal_EventFirstInternalApproach)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    int caller = CALLER_EVENT;
    std::atomic<int> loaded;
    std::atomic<int>& ref = loaded;
    
    MockIVSAMapInternal* mockIVSAMapInternal = new MockIVSAMapInternal;
    EXPECT_CALL(*mockIVSAMapInternal, EnableInternalAccess).WillRepeatedly(Return(0));
    loaded = LOADING;
    EXPECT_CALL(*mockIVSAMapInternal, GetLoadDoneFlag).WillRepeatedly(ReturnRef(ref));

    VSAMapAPI sut(mockIVSAMapInternal, nullptr);

    // When
    VirtualBlkAddr rvsa = sut.GetVSAInternal(VOLID, START_RBA, caller);

    // Then
    EXPECT_EQ(rvsa, UNMAP_VSA);
    EXPECT_EQ(caller, NEED_RETRY);
    delete mockIVSAMapInternal;
}

TEST(VSAMapAPI, GetVSAInternal_EventDeletedVolume)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    int caller = CALLER_EVENT;
    std::atomic<int> loaded;
    std::atomic<int>& ref = loaded;
    
    MockIVSAMapInternal* mockIVSAMapInternal = new MockIVSAMapInternal;
    EXPECT_CALL(*mockIVSAMapInternal, EnableInternalAccess).WillRepeatedly(Return(-EID(VSAMAP_LOAD_FAILURE)));
    loaded = LOADING;
    EXPECT_CALL(*mockIVSAMapInternal, GetLoadDoneFlag).WillRepeatedly(ReturnRef(ref));

    VSAMapAPI sut(mockIVSAMapInternal, nullptr);

    // When
    VirtualBlkAddr rvsa = sut.GetVSAInternal(VOLID, START_RBA, caller);

    // Then
    EXPECT_EQ(rvsa, UNMAP_VSA);
    // EXPECT_EQ(caller, NEED_RETRY);
    delete mockIVSAMapInternal;
}

TEST(VSAMapAPI, SetVSAsInternal_Refused)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    const uint32_t NUM_BLKS = 3;
    VirtualBlkAddr vsa {.stripeId = 1, .offset = 2};    
    VirtualBlks vsas {.startVsa = vsa, .numBlks = NUM_BLKS};

    MockIVSAMapInternal* mockIVSAMapInternal = new MockIVSAMapInternal;
    EXPECT_CALL(*mockIVSAMapInternal, EnableInternalAccess).WillRepeatedly(Return(-1));

    VSAMapAPI sut(mockIVSAMapInternal, nullptr);

    // When
    int reti = sut.SetVSAsInternal(VOLID, START_RBA, vsas);

    // Then
    EXPECT_EQ(reti, -1);
    delete mockIVSAMapInternal;
}

TEST(VSAMapAPI, SetVSAsInternal_Default)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    const uint32_t NUM_BLKS = 3;
    VirtualBlkAddr vsa {.stripeId = 1, .offset = 2};    
    VirtualBlks vsas {.startVsa = vsa, .numBlks = NUM_BLKS};

    MockIVSAMapInternal* mockIVSAMapInternal = new MockIVSAMapInternal;
    EXPECT_CALL(*mockIVSAMapInternal, EnableInternalAccess).WillRepeatedly(Return(0));

    MockVSAMapContent* mockVSAMapContent = new MockVSAMapContent;
    EXPECT_CALL(*mockVSAMapContent, SetEntry).WillRepeatedly(Return(0));

    VSAMapAPI sut(mockIVSAMapInternal, nullptr);
    VSAMapContent*& vsaMapContent = sut.GetVSAMapContent(VOLID);   // Get Reference
    vsaMapContent = mockVSAMapContent; // Replace with Mock

    // When
    int reti = sut.SetVSAsInternal(VOLID, START_RBA, vsas);

    // Then
    EXPECT_EQ(reti, 0);
    delete mockIVSAMapInternal;
    vsaMapContent = nullptr;    // Replace nullptr again, due to vsaMapAPISUT Dtor crash    
    delete mockVSAMapContent;
}

TEST(VSAMapAPI, GetRandomVSA_Default)
{
    // Given
    BlkAddr rba = 1024;

    MockMapperAddressInfo* mockMapperAddressInfo = new MockMapperAddressInfo(nullptr);
    mockMapperAddressInfo->blksPerStripe = 128;

    VSAMapAPI sut(nullptr, mockMapperAddressInfo);
    VirtualBlkAddr vsa = {.stripeId = (StripeId)rba / 128, .offset = rba % 128};

    // When
    VirtualBlkAddr rvsa = sut.GetRandomVSA(rba);

    // Then
    EXPECT_EQ(rvsa, vsa);
}

TEST(VSAMapAPI, GetDirtyVsaMapPages_Default)
{
    // Given
    const int VOLID = 0;
    const BlkAddr START_RBA = 0;
    const uint32_t NUM_BLKS = 3;

    MpageList mplist;
    MockVSAMapContent* mockVSAMapContent = new MockVSAMapContent;
    EXPECT_CALL(*mockVSAMapContent, GetDirtyPages).WillRepeatedly(Return(mplist));

    VSAMapAPI sut(nullptr, nullptr);
    VSAMapContent*& vsaMapContent = sut.GetVSAMapContent(VOLID);   // Get Reference
    vsaMapContent = mockVSAMapContent; // Replace with Mock

    // When
    MpageList rmplist = sut.GetDirtyVsaMapPages(VOLID, START_RBA, NUM_BLKS);

    // Then
    EXPECT_EQ(rmplist, mplist);
    vsaMapContent = nullptr;
    delete mockVSAMapContent;
}

TEST(VSAMapAPI, GetNumUsedBlocks_vsaMapIsNullPtr)
{
    // Given
    const int VOLID = 0;
    int reti = 0;
    VSAMapAPI sut(nullptr, nullptr);

    // When
    reti = sut.GetNumUsedBlocks(VOLID);

    // Then
    EXPECT_EQ(reti, -(int64_t)POS_EVENT_ID::VSAMAP_NULL_PTR);
}

TEST(VSAMapAPI, GetNumUsedBlocks_vsaMapIsValid)
{
    // Given
    int reti = 0;
    const int VOLID = 0;
    const int64_t NUMBLK = 10;

    MockVSAMapContent* mockVSAMapContent = new MockVSAMapContent;
    EXPECT_CALL(*mockVSAMapContent, GetNumUsedBlocks).WillRepeatedly(Return(NUMBLK));

    VSAMapAPI sut(nullptr, nullptr);
    VSAMapContent*& vsaMapContent = sut.GetVSAMapContent(VOLID);   // Get Reference
    vsaMapContent = mockVSAMapContent; // Replace with Mock

    // When
    reti = sut.GetNumUsedBlocks(VOLID);

    // Then
    EXPECT_EQ(reti, NUMBLK);
    vsaMapContent = nullptr;    // Replace nullptr again, due to vsaMapAPISUT Dtor crash
    delete mockVSAMapContent;
}

TEST(VSAMapAPI, _UpdateVsaMap_)
{
}

TEST(VSAMapAPI, _ReadVSA_)
{
}

TEST(VSAMapAPI, _IsVSAMapLoaded_)
{
}

TEST(VSAMapAPI, _IsVsaMapAccessible_)
{
}

} // namespace pos
