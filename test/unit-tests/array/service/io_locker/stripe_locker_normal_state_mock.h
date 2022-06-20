#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_locker/stripe_locker_normal_state.h"

namespace pos
{
class MockStripeLockerNormalState : public StripeLockerNormalState
{
public:
    using StripeLockerNormalState::StripeLockerNormalState;
    MOCK_METHOD(bool, TryLock, (StripeLockInfo val), (override));
    MOCK_METHOD(void, Unlock, (StripeId val), (override));
    MOCK_METHOD(bool, Exists, (StripeId val), (override));
    MOCK_METHOD(uint32_t, Count, (), (override));
};

} // namespace pos
