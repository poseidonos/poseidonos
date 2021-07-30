#include "src/metafs/mvm/volume/ssd/ssd_meta_volume.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_manager_mock.h"
#include "test/unit-tests/metafs/mvm/volume/catalog_manager_mock.h"
#include <string>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class SsdMetaVolumeTestFixture : public ::testing::Test
{
public:
    SsdMetaVolumeTestFixture(void)
    : inodeMgr(nullptr),
      catalogMgr(nullptr)
    {
    }
    virtual ~SsdMetaVolumeTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        inodeMgr = new NiceMock<InodeManager>(arrayId);
        catalogMgr = new NiceMock<CatalogManager>(arrayId);

        volume = new SsdMetaVolume(arrayId, maxLpn, inodeMgr, catalogMgr);
        volume->Init(metaStorage);
        volume->InitVolumeBaseLpn();
    }

    virtual void
    TearDown(void)
    {
        delete volume;
    }

protected:
    SsdMetaVolume* volume;

    NiceMock<InodeManager>* inodeMgr;
    NiceMock<CatalogManager>* catalogMgr;

    int arrayId = 0;
    MetaLpnType maxLpn = 8192;
    MockMetaStorageSubsystem* metaStorage;
    MetaFilePropertySet prop;
};

TEST_F(SsdMetaVolumeTestFixture, SSD_Meta_Volume_Normal)
{
    // then
    // the maximum size
    EXPECT_TRUE(volume->IsOkayToStore(4087 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // expected count of the free lpn: 4088
    EXPECT_TRUE(volume->IsOkayToStore(4088 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // more than possible
    EXPECT_FALSE(volume->IsOkayToStore(4089 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
}

} // namespace pos
