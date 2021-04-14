#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/lib/timeout_checker.h"

namespace pos
{
class MockTimeoutChecker : public TimeoutChecker
{
public:
    using TimeoutChecker::TimeoutChecker;
    MOCK_METHOD(void, SetTimeout, (uint64_t intervalFromNow), (override));
    MOCK_METHOD(bool, CheckTimeout, (), (override));
};

} // namespace pos
