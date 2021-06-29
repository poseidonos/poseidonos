#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/lib/system_timeout_checker.h"

namespace pos
{
class MockSystemTimeoutChecker : public SystemTimeoutChecker
{
public:
    using SystemTimeoutChecker::SystemTimeoutChecker;
    MOCK_METHOD(void, SetTimeout, (uint64_t nanoSecsLeftFromNow), (override));
    MOCK_METHOD(bool, CheckTimeout, (), (override));
};

} // namespace pos
