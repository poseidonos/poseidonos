#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_wbstripe_internal.h"

namespace pos
{
class MockIWBStripeInternal : public IWBStripeInternal
{
public:
    using IWBStripeInternal::IWBStripeInternal;
    MOCK_METHOD(Stripe*, GetStripe, (StripeId wbLsid), (override));
};

} // namespace pos
