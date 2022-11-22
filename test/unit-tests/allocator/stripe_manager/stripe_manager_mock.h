#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/stripe_manager/stripe_manager.h"

namespace pos
{
class MockStripeManager : public StripeManager
{
public:
    using StripeManager::StripeManager;
    MOCK_METHOD(void, Init, (IWBStripeAllocator * wbStripeManager), (override));
    MOCK_METHOD((std::pair<StripeId, StripeId>), AllocateStripesForUser, (uint32_t volumeId), (override));
    MOCK_METHOD(StripeSmartPtr, AllocateGcDestStripe, (uint32_t volumeId), (override));
};

} // namespace pos
