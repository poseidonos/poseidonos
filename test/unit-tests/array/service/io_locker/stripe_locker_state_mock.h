#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_locker/stripe_locker_state.h"

namespace pos
{
class MockStripeLockerState : public StripeLockerState
{
public:
    using StripeLockerState::StripeLockerState;
    MOCK_METHOD(bool, TryLock, (StripeId val), (override));
    MOCK_METHOD(void, Unlock, (StripeId val), (override));
    MOCK_METHOD(bool, StateChange, (LockerMode mode), (override));
    MOCK_METHOD(uint32_t, Count, (), (override));
};

} // namespace pos
