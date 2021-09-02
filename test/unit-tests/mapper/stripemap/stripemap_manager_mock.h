#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/stripemap/stripemap_manager.h"

namespace pos
{
class MockStripeMapManager : public StripeMapManager
{
public:
    using StripeMapManager::StripeMapManager;
    MOCK_METHOD(void, MapFlushDone, (int mapId), (override));
    MOCK_METHOD(StripeId, GetRandomLsid, (StripeId vsid), (override));
};

} // namespace pos
