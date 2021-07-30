#include "src/metafs/mvm/volume/catalog_manager.h"
#include "test/unit-tests/metafs/mvm/volume/catalog_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::_;
using testing::NiceMock;
using testing::Return;
namespace pos
{
TEST(CatalogManager, CreateObject)
{
    CatalogManager* catalogMgr = new CatalogManager(0);

    catalogMgr->Init(MetaVolumeType::SsdVolume, 0, 100);

    delete catalogMgr;
}

TEST(CatalogManager, UnusedMethod)
{
    CatalogManager* catalogMgr = new CatalogManager(0);

    catalogMgr->Bringup();
    catalogMgr->Finalize();

    delete catalogMgr;
}

TEST(CatalogManager, CreateObject_New)
{
    NiceMock<MockCatalog>* catalog = new NiceMock<MockCatalog>(MetaVolumeType::SsdVolume, 0);
    CatalogManager* catalogMgr = new CatalogManager(catalog, 0);

    catalogMgr->Init(MetaVolumeType::SsdVolume, 0, 100);

    EXPECT_EQ(catalogMgr->GetRegionSizeInLpn(), 1);

    delete catalogMgr;
}

} // namespace pos
