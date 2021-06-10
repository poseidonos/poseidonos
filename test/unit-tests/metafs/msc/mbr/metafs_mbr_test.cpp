#include "src/metafs/msc/mbr/metafs_mbr.h"

#include <gtest/gtest.h>

namespace pos
{
} // namespace pos

namespace pos
{
TEST(MetaFsMBR, CheckSignature)
{
    MetaFsAnchorRegionType regionType = MetaFsAnchorRegionType::MasterBootRecord;
    MetaLpnType baseLpn = 0;
    MetaFsMBR* mbr = new MetaFsMBR(regionType, baseLpn);

    mbr->CreateMBR();

    // empty method
    mbr->BuildMBR();

    EXPECT_NE(mbr->GetEpochSignature(), 0);

    delete mbr;
}

TEST(MetaFsMBR, CheckValidity)
{
    MetaFsAnchorRegionType regionType = MetaFsAnchorRegionType::MasterBootRecord;
    MetaLpnType baseLpn = 0;
    MetaFsMBR* mbr = new MetaFsMBR(regionType, baseLpn);

    mbr->CreateMBR();

    EXPECT_TRUE(mbr->IsValidMBRExist());

    delete mbr;
}

TEST(MetaFsMBR, CheckSpor)
{
    MetaFsAnchorRegionType regionType = MetaFsAnchorRegionType::MasterBootRecord;
    MetaLpnType baseLpn = 0;
    MetaFsMBR* mbr = new MetaFsMBR(regionType, baseLpn);

    mbr->CreateMBR();

    mbr->SetPORStatus(true);

    EXPECT_TRUE(mbr->GetPORStatus());

    mbr->SetPORStatus(false);

    EXPECT_FALSE(mbr->GetPORStatus());

    delete mbr;
}

TEST(MetaFsMBR, Invalidation)
{
    MetaFsAnchorRegionType regionType = MetaFsAnchorRegionType::MasterBootRecord;
    MetaLpnType baseLpn = 0;
    MetaFsMBR* mbr = new MetaFsMBR(regionType, baseLpn);

    mbr->CreateMBR();

    EXPECT_TRUE(mbr->IsValidMBRExist());

    mbr->InvalidMBRSignature();

    EXPECT_FALSE(mbr->IsValidMBRExist());

    delete mbr;
}

} // namespace pos
