#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>
#include <set>

#include "src/array/service/io_locker/i_io_locker.h"

namespace pos
{
class MockIIOLocker : public IIOLocker
{
public:
    using IIOLocker::IIOLocker;
    MOCK_METHOD(bool, TryLock, (IArrayDevice* dev, StripeId val), (override));
    MOCK_METHOD(bool, TryLock, (set<IArrayDevice*>& devs, StripeId val), (override));
    MOCK_METHOD(void, Unlock, (IArrayDevice* dev, StripeId val), (override));
    MOCK_METHOD(void, Unlock, (set<IArrayDevice*>& devs, StripeId val), (override));
    MOCK_METHOD(bool, TryChange, (IArrayDevice* dev, LockerMode mode), (override));
};

} // namespace pos
