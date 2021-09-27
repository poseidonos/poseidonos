#include "src/metafs/mvm/volume/on_volume_meta_region.h"
#include "src/metafs/mvm/meta_region_type.h"
#include "src/metafs/mvm/volume/catalog.h"
#include "src/metafs/mvm/volume/inode_table.h"
#include "src/metafs/mvm/volume/inode_table_header.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(OnVolumeMetaRegion, Construct_Destruct)
{
    OnVolumeMetaRegion<MetaRegionType, CatalogContent>* a =
        new OnVolumeMetaRegion<MetaRegionType, CatalogContent>
            (MetaVolumeType::SsdVolume, MetaRegionType::VolCatalog, 0);
    OnVolumeMetaRegion<MetaRegionType, InodeTableContent>* b =
        new OnVolumeMetaRegion<MetaRegionType, InodeTableContent>
            (MetaVolumeType::SsdVolume, MetaRegionType::FileInodeTable, 0);
    OnVolumeMetaRegion<MetaRegionType, InodeTableHeaderContent>* c =
        new OnVolumeMetaRegion<MetaRegionType, InodeTableHeaderContent>
            (MetaVolumeType::SsdVolume, MetaRegionType::FileInodeHdr, 0);

    delete a;
    delete b;
    delete c;
}
} // namespace pos
