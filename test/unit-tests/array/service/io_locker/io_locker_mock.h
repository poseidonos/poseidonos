#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_locker/io_locker.h"

namespace pos
{
class MockIOLocker : public IOLocker
{
public:
    using IOLocker::IOLocker;
    MOCK_METHOD(bool, TryLock, (string array, StripeId val), (override));
    MOCK_METHOD(void, Unlock, (string array, StripeId val), (override));
    MOCK_METHOD(bool, TryChange, (string array, LockerMode mode), (override));
};

} // namespace pos
