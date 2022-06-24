#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_locker/stripe_locker_busy_state.h"

namespace pos
{
class MockStripeLockerBusyState : public StripeLockerBusyState
{
public:
    using StripeLockerBusyState::StripeLockerBusyState;
    MOCK_METHOD(bool, TryLock, (StripeLockInfo val), (override));
    MOCK_METHOD(void, Unlock, (StripeId val), (override));
    MOCK_METHOD(bool, Exists, (StripeId val), (override));
    MOCK_METHOD(uint32_t, Count, (), (override));
};

} // namespace pos
