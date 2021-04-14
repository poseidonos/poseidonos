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
    MOCK_METHOD(bool, TryLock, (StripeId val), (override));
    MOCK_METHOD(void, Unlock, (StripeId val), (override));
    MOCK_METHOD(bool, StateChange, (LockerMode mode), (override));
};

} // namespace pos
