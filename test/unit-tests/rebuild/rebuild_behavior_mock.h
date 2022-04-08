#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/rebuild/rebuild_behavior.h"

namespace pos
{
class MockRebuildBehavior : public RebuildBehavior
{
public:
    using RebuildBehavior::RebuildBehavior;
    MOCK_METHOD(bool, Read, (), (override));
    MOCK_METHOD(bool, Write, (uint32_t targetId, UbioSmartPtr ubio), (override));
    MOCK_METHOD(bool, Complete, (uint32_t targetId, UbioSmartPtr ubio), (override));
    MOCK_METHOD(void, UpdateProgress, (uint32_t val), (override));
    MOCK_METHOD(string, _GetClassName, (), (override));
};

} // namespace pos
