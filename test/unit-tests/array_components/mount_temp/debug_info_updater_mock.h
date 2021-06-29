#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_components/mount_temp/debug_info_updater.h"

namespace pos
{
class MockDebugInfoUpdater : public DebugInfoUpdater
{
public:
    using DebugInfoUpdater::DebugInfoUpdater;
    MOCK_METHOD(void, Update, (), (override));
};

} // namespace pos
