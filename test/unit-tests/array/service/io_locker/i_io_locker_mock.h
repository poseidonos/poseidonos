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
    MOCK_METHOD(bool, TryBusyLock, (IArrayDevice* dev, StripeId from, StripeId to), (override));
    MOCK_METHOD(bool, TryBusyLock, (set<IArrayDevice*>& devs, StripeId from, StripeId to, IArrayDevice*& failed), (override));
    MOCK_METHOD(bool, ResetBusyLock, (IArrayDevice* dev, bool forceReset), (override));
    MOCK_METHOD(bool, TryLock, (set<IArrayDevice*>& devs, StripeId val), (override));
    MOCK_METHOD(void, Unlock, (IArrayDevice* dev, StripeId val), (override));
    MOCK_METHOD(void, Unlock, (set<IArrayDevice*>& devs, StripeId val), (override));
    MOCK_METHOD(void, WriteBusyLog, (IArrayDevice* dev), (override));
};

} // namespace pos
