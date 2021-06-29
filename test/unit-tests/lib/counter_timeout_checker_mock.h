#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/lib/counter_timeout_checker.h"

namespace pos
{
class MockCounterTimeoutChecker : public CounterTimeoutChecker
{
public:
    using CounterTimeoutChecker::CounterTimeoutChecker;
    MOCK_METHOD(void, SetTimeout, (uint64_t countLeftFromNow), (override));
    MOCK_METHOD(bool, CheckTimeout, (), (override));
};

} // namespace pos
