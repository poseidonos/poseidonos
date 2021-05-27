#include "src/allocator/context_manager/active_stripe_index_info.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ActiveStripeTailArrIdxInfo, ActiveStripeTailArrIdxInfo_TestConstructorAndDistructor)
{
    ActiveStripeTailArrIdxInfo* info = new ActiveStripeTailArrIdxInfo(0, true);
    delete info;
}

TEST(ActiveStripeTailArrIdxInfo, GetActiveStripeTailArrIdx_TestGetDataWithGcTrue)
{
    // given 1.
    ActiveStripeTailArrIdxInfo* info = new ActiveStripeTailArrIdxInfo(1, true);
    // when 1.
    int ret = info->GetActiveStripeTailArrIdx();
    // then 1.
    EXPECT_EQ(MAX_VOLUME_COUNT + 1, ret);
    delete info;
}

TEST(ActiveStripeTailArrIdxInfo, GetActiveStripeTailArrIdx_TestGetDataWithGcFalse)
{
    // given 1.
    ActiveStripeTailArrIdxInfo* info = new ActiveStripeTailArrIdxInfo(1, false);
    // when 1.
    int ret = info->GetActiveStripeTailArrIdx();
    // then 1.
    EXPECT_EQ(1, ret);
    delete info;
}

TEST(ActiveStripeTailArrIdxInfo, GetVolumeId_TestGetUserVolId)
{
    // given 1.
    ActiveStripeTailArrIdxInfo* info = new ActiveStripeTailArrIdxInfo(1, false);
    // when 1.
    int ret = info->GetVolumeId(MAX_VOLUME_COUNT + 1);
    // then 1.
    EXPECT_EQ(1, ret);
    delete info;
}

TEST(ActiveStripeTailArrIdxInfo, GetVolumeId_TestGetGcVolId)
{
    // given 1.
    ActiveStripeTailArrIdxInfo* info = new ActiveStripeTailArrIdxInfo(5, false);
    // when 1.
    int ret = info->GetVolumeId(5);
    // then 1.
    EXPECT_EQ(5, ret);
    delete info;
}

TEST(ActiveStripeTailArrIdxInfo, IsGc_TestGcTrue)
{
    // given 1.
    ActiveStripeTailArrIdxInfo* info = new ActiveStripeTailArrIdxInfo(1, true);
    // when 1.
    bool ret = info->IsGc(MAX_VOLUME_COUNT + 1);
    // then 1.
    EXPECT_EQ(true, ret);
    delete info;
}

TEST(ActiveStripeTailArrIdxInfo, IsGc_TestGcFalse)
{
    // given 1.
    ActiveStripeTailArrIdxInfo* info = new ActiveStripeTailArrIdxInfo(1, true);
    // when 1.
    bool ret = info->IsGc(1);
    // then 1.
    EXPECT_EQ(false, ret);
    delete info;
}

} // namespace pos
