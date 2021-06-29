#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/volume_catalog.h"

namespace pos
{
class MockVolumeCatalogContent : public VolumeCatalogContent
{
public:
    using VolumeCatalogContent::VolumeCatalogContent;
};

class MockVolumeCatalog : public VolumeCatalog
{
public:
    using VolumeCatalog::VolumeCatalog;
};

} // namespace pos
