#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/debug/debug_info_updater.h"

namespace pos
{
class MockDebugInfoUpdater : public DebugInfoUpdater
{
public:
    using DebugInfoUpdater::DebugInfoUpdater;
    MOCK_METHOD(void, Update, (), (override));
};

} // namespace pos
