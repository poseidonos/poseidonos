#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_models/interface/i_array_state_subscription.h"

namespace pos
{
class MockIArrayStateSubscription : public IArrayStateSubscription
{
public:
    using IArrayStateSubscription::IArrayStateSubscription;
    MOCK_METHOD(void, Register, (IArrayStateSubscriber * subscriber), (override));
    MOCK_METHOD(void, Unregister, (IArrayStateSubscriber * subscriber), (override));
};

} // namespace pos
