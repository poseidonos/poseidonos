#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/debug/debug_info.h"

namespace pos
{
class MockDebugInfo : public DebugInfo
{
public:
    using DebugInfo::DebugInfo;
    MOCK_METHOD(void, Update, (), (override));
};

} // namespace pos
