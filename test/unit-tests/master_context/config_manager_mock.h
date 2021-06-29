#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/master_context/config_manager.h"

namespace pos
{
class MockConfigManager : public ConfigManager
{
public:
    using ConfigManager::ConfigManager;
    MOCK_METHOD(int, ReadFile, (), (override));
    MOCK_METHOD(int, GetValue, (string module, string key, void* value, ConfigType type), (override));
};

} // namespace pos
