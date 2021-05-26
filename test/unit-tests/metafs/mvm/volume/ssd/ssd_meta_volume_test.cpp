#include "src/metafs/mvm/volume/ssd/ssd_meta_volume.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include "test/unit-tests/metafs/mvm/volume/meta_file_manager_mock.h"
#include "test/unit-tests/metafs/mvm/volume/mf_inode_mgr_mock.h"
#include "test/unit-tests/metafs/mvm/volume/volume_catalog_manager_mock.h"
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
    : fileMgr(nullptr),
      inodeMgr(nullptr),
      catalogMgr(nullptr)
    {
    }
    virtual ~SsdMetaVolumeTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        fileMgr = new NiceMock<MetaFileManager>(arrayName);
        inodeMgr = new NiceMock<MetaFileInodeManager>(arrayName);
        catalogMgr = new NiceMock<VolumeCatalogManager>(arrayName);

        volume = new SsdMetaVolume(fileMgr, inodeMgr, catalogMgr, arrayName, maxLpn);
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

    NiceMock<MetaFileManager>* fileMgr;
    NiceMock<MetaFileInodeManager>* inodeMgr;
    NiceMock<VolumeCatalogManager>* catalogMgr;

    std::string arrayName = "TESTARRAY";
    MetaLpnType maxLpn = 8192;
    MockMetaStorageSubsystem* metaStorage;
    MetaFilePropertySet prop;
};

TEST_F(SsdMetaVolumeTestFixture, SSD_Meta_Volume_Normal)
{
    // then
    // the maximum size
    EXPECT_TRUE(volume->IsOkayToStore(4089 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // expected count of the free lpn: 4090
    EXPECT_FALSE(volume->IsOkayToStore(4090 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // more than possible
    EXPECT_FALSE(volume->IsOkayToStore(4091 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
}

} // namespace pos
