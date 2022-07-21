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

template<typename StateT>
class MockChangeLogger : public ChangeLogger<StateT>
{
public:
    using ChangeLogger<StateT>::ChangeLogger;
    MOCK_METHOD(void, LoggingStateChangeConditionally,
        (spdlog::source_loc loc, spdlog::level::level_enum lvl, int id,
            StateT currentState, std::string msg));
};
} // namespace pos
