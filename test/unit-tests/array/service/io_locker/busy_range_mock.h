#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>
#include <set>

#include "src/array/service/io_locker/busy_range.h"

namespace pos
{
class MockBusyRange : public BusyRange
{
public:
    using BusyRange::BusyRange;
    MOCK_METHOD(void, SetBusyRange, (StripeId from, StripeId to), (override));
    MOCK_METHOD(bool, IsBusy, (StripeId val), (override));
};

}  // namespace pos
