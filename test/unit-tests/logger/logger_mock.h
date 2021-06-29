#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/logger/logger.h"

namespace pos
{
class MockLogger : public Logger
{
public:
    using Logger::Logger;
};

class MockReporter : public Reporter
{
public:
    using Reporter::Reporter;
};

} // namespace pos
