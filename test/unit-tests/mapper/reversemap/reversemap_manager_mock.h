#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/reversemap/reversemap_manager.h"

namespace pos
{
class MockReverseMapManager : public ReverseMapManager
{
public:
    using ReverseMapManager::ReverseMapManager;
    MOCK_METHOD(int, LinkReverseMap, (Stripe * stripe, StripeId wbLsid, StripeId vsid), (override));
    MOCK_METHOD(ReverseMapPack*, AllocReverseMapPack, (), (override));
};

} // namespace pos
