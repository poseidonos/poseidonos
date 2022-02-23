#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/rebuild/raid1_rebuild.h"

namespace pos
{
class MockRaid1Rebuild : public Raid1Rebuild
{
public:
    using Raid1Rebuild::Raid1Rebuild;
    MOCK_METHOD(bool, Read, (), (override));
    MOCK_METHOD(bool, Write, (uint32_t targetId, UbioSmartPtr ubio), (override));
    MOCK_METHOD(bool, Complete, (uint32_t targetId, UbioSmartPtr ubio), (override));
    MOCK_METHOD(void, UpdateProgress, (uint32_t val), (override));
    MOCK_METHOD(string, _GetClassName, (), (override));
    MOCK_METHOD(int, _GetTotalReadChunksForRecovery, (), (override));
};

} // namespace pos
