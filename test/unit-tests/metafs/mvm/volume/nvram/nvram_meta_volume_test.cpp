#include "src/metafs/mvm/volume/nvram/nvram_meta_volume.h"
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
class NvRamMetaVolumeTestFixture : public ::testing::Test
{
public:
    NvRamMetaVolumeTestFixture(void)
    : fileMgr(nullptr),
      inodeMgr(nullptr),
      catalogMgr(nullptr)
    {
    }
    virtual ~NvRamMetaVolumeTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        fileMgr = new NiceMock<MetaFileManager>(arrayName);
        inodeMgr = new NiceMock<MetaFileInodeManager>(arrayName);
        catalogMgr = new NiceMock<VolumeCatalogManager>(arrayName);

        volume = new NvRamMetaVolume(fileMgr, inodeMgr, catalogMgr, arrayName, maxLpn);
        volume->Init(metaStorage);
        volume->InitVolumeBaseLpn();
    }

    virtual void
    TearDown(void)
    {
        delete volume;
    }

protected:
    NvRamMetaVolume* volume;

    NiceMock<MetaFileManager>* fileMgr;
    NiceMock<MetaFileInodeManager>* inodeMgr;
    NiceMock<VolumeCatalogManager>* catalogMgr;

    std::string arrayName = "TESTARRAY";
    MetaLpnType maxLpn = 8192;
    MockMetaStorageSubsystem* metaStorage;
    MetaFilePropertySet prop;
};

TEST_F(NvRamMetaVolumeTestFixture, NVRAM_Meta_Volume_Normal)
{
    // then
    // prop is not suitable
    // the maximum size
    EXPECT_FALSE(volume->IsOkayToStore(6141 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // expected count of the free lpn: 6142
    EXPECT_FALSE(volume->IsOkayToStore(6142 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // more than possible
    EXPECT_FALSE(volume->IsOkayToStore(6143 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));

    // prop is suitable
    prop.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
    EXPECT_TRUE(volume->IsOkayToStore(6141 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // expected count of the free lpn: 6142
    EXPECT_TRUE(volume->IsOkayToStore(6142 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // more than possible
    EXPECT_FALSE(volume->IsOkayToStore(6143 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));

    prop.ioAccPattern = MetaFileAccessPattern::SmallSizeBlockIO;
    EXPECT_TRUE(volume->IsOkayToStore(6141 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // expected count of the free lpn: 6142
    EXPECT_TRUE(volume->IsOkayToStore(6142 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // more than possible
    EXPECT_FALSE(volume->IsOkayToStore(6143 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));

    prop.ioOpType = MetaFileDominant::WriteDominant;
    EXPECT_TRUE(volume->IsOkayToStore(6141 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // expected count of the free lpn: 6142
    EXPECT_TRUE(volume->IsOkayToStore(6142 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
    // more than possible
    EXPECT_FALSE(volume->IsOkayToStore(6143 * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE, prop));
}

} // namespace pos
