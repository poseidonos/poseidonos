#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/catalog.h"

namespace pos
{
class MockCatalog : public Catalog
{
public:
    using Catalog::Catalog;
    MOCK_METHOD(void, Create, (MetaLpnType maxVolumeLpn, uint32_t maxFileNumSupport));
    MOCK_METHOD(void, RegisterRegionInfo, (MetaRegionType regionType, MetaLpnType baseLpn, MetaLpnType maxLpn));
    MOCK_METHOD(bool, CheckValidity, ());
};

} // namespace pos
