#include "src/metafs/mvm/volume/on_volume_meta_region_mgr.h"

#include <gtest/gtest.h>

namespace pos
{
class OnVolumeMetaRegionManagerTest : public OnVolumeMetaRegionManager
{
public:
    explicit OnVolumeMetaRegionManagerTest(int arrayId)
    : OnVolumeMetaRegionManager(arrayId)
    {
    }

    ~OnVolumeMetaRegionManagerTest(void)
    {
    }

    void Bringup(void)
    {
    }

    bool SaveContent(void)
    {
        return true;
    }

    MetaLpnType GetRegionSizeInLpn(void)
    {
        return 0;
    }

    void Finalize(void)
    {
    }

    void SetMss(MetaStorageSubsystem* metaStorage)
    {
    }
};

TEST(OnVolumeMetaRegionManagerTest, Init0)
{
    OnVolumeMetaRegionManagerTest* obj
        = new OnVolumeMetaRegionManagerTest(0);

    obj->Init(MetaVolumeType::SsdVolume, 0, 0);

    delete obj;
}

TEST(OnVolumeMetaRegionManagerTest, Init1)
{
    OnVolumeMetaRegionManagerTest* obj
        = new OnVolumeMetaRegionManagerTest(0);

    obj->Init(MetaStorageType::SSD, 0, 0);

    delete obj;
}

TEST(OnVolumeMetaRegionManagerTest, Init2)
{
    OnVolumeMetaRegionManagerTest obj(0);

    obj.Init(MetaVolumeType::SsdVolume, 0, 0);
}

TEST(OnVolumeMetaRegionManagerTest, Init3)
{
    OnVolumeMetaRegionManagerTest obj(0);

    obj.Init(MetaStorageType::SSD, 0, 0);
}

} // namespace pos
