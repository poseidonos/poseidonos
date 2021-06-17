#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_locker/i_io_locker.h"

namespace pos
{
class MockIIOLocker : public IIOLocker
{
public:
    using IIOLocker::IIOLocker;
    MOCK_METHOD(bool, TryLock, (string array, StripeId val), (override));
    MOCK_METHOD(void, Unlock, (string array, StripeId val), (override));
    MOCK_METHOD(bool, TryChange, (string array, LockerMode mode), (override));
    MOCK_METHOD(bool, TryLock, (unsigned int arrayIndex, StripeId val), (override));
    MOCK_METHOD(void, Unlock, (unsigned int arrayIndex, StripeId val), (override));
    MOCK_METHOD(bool, TryChange, (unsigned int arrayIndex, LockerMode mode), (override));
};

} // namespace pos
