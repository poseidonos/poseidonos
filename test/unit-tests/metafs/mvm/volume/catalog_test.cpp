#include "src/metafs/mvm/volume/catalog.h"

#include <gtest/gtest.h>
 // namespace pos
namespace pos
{
class CatalogTester : public Catalog
{
public:
    CatalogTester(MetaVolumeType volumeType, MetaLpnType baseLpn)
    : Catalog(volumeType, baseLpn)
    {
    }

    uint64_t GetSignature(void)
    {
        return VOLUME_CATALOG_SIGNATURE;
    }
};

TEST(Catalog, CreateObject)
{
    MetaLpnType maxVolumeLpn = 1024;
    uint32_t maxFileNumSupport = 30;

    CatalogTester* catalog = new CatalogTester(MetaVolumeType::SsdVolume, 0);
    CatalogContent* content = catalog->GetContent();

    catalog->Create(maxVolumeLpn, maxFileNumSupport);

    EXPECT_EQ(content->volumeInfo.maxVolPageNum, maxVolumeLpn);
    EXPECT_EQ(content->volumeInfo.maxFileNumSupport, maxFileNumSupport);
    EXPECT_EQ(content->signature, catalog->GetSignature());

    EXPECT_TRUE(catalog->CheckValidity());

    delete catalog;
}

TEST(Catalog, RegisterRegion)
{
    MetaLpnType maxVolumeLpn = 1024;
    uint32_t maxFileNumSupport = 30;

    CatalogTester* catalog = new CatalogTester(MetaVolumeType::SsdVolume, 0);
    CatalogContent* content = catalog->GetContent();

    catalog->Create(maxVolumeLpn, maxFileNumSupport);

    for (int i = 0; i < (int)MetaRegionType::Max; ++i)
    {
        catalog->RegisterRegionInfo((MetaRegionType)i, i * 5 + 1, i * 5 + 5);
    }

    for (int i = 0; i < (int)MetaRegionType::Max; ++i)
    {
        EXPECT_EQ(content->regionMap[i].baseLpn, i * 5 + 1);
        EXPECT_EQ(content->regionMap[i].maxLpn,  i * 5 + 5);
    }

    delete catalog;
}
} // namespace pos
