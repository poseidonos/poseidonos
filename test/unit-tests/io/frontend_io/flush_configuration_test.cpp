#include "src/io/frontend_io/flush_configuration.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "test/unit-tests/master_context/config_manager_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FlushConfiguration, FlushConfiguration_Constructor_ZeroArgument_Stack)
{
    // Given
    // When: create flushConfiguration
    FlushConfiguration flushConfiguration;
    // Then: do nothing
}

TEST(FlushConfiguration, FlushConfiguration_Constructor_ZeroArgument_Heap)
{
    // Given
    // When: create flushConfiguration
    FlushConfiguration* flushConfiguration = new FlushConfiguration();
    delete flushConfiguration;
    // Then: do nothing
}

TEST(FlushConfiguration, FlushConfiguration_Constructor_OneArgument_Stack)
{
    // Given
    NiceMock<MockConfigManager> mockConfigManager;
    int call_count{0};
    bool actual, expected;

    ON_CALL(mockConfigManager, GetValue).WillByDefault([&mockConfigManager, &call_count](string module, string key, void* value, ConfigType type) {
        cout << call_count << endl;
        switch (call_count)
        {
            case 0:
            case 1:
                if (0 == module.compare("flush"))
                {
                    *(bool*)value = false;
                }
                else if (0 == module.compare("journal"))
                {
                    *(bool*)value = false;
                }
                break;
            case 2:
            case 3:
                if (0 == module.compare("flush"))
                {
                    *(bool*)value = true;
                }
                else if (0 == module.compare("journal"))
                {
                    *(bool*)value = false;
                }
                break;
            case 4:
            case 5:
                if (0 == module.compare("flush"))
                {
                    *(bool*)value = false;
                }
                else if (0 == module.compare("journal"))
                {
                    *(bool*)value = true;
                }
                break;
            case 6:
            case 7:
                if (0 == module.compare("flush"))
                {
                    *(bool*)value = true;
                }
                else if (0 == module.compare("journal"))
                {
                    *(bool*)value = true;
                }
                break;
            default:
                return -1;
        }
        cout << "val:" << *(bool*)value << endl;
        call_count++;
        return 0;
    });

    // When: set flush as false and journal as false
    FlushConfiguration flushConfiguration1(&mockConfigManager);

    // Then: IsEnabled() has to return false
    actual = flushConfiguration1.IsEnabled();
    expected = false;
    ASSERT_EQ(expected, actual);

    // When: set flush as true and journal as false
    FlushConfiguration flushConfiguration2(&mockConfigManager);

    // Then: IsEnabled() has to return true
    actual = flushConfiguration2.IsEnabled();
    expected = true;
    ASSERT_EQ(expected, actual);

    // When: set flush as false and journal as true
    FlushConfiguration flushConfiguration3(&mockConfigManager);

    // Then: IsEnabled() has to return false
    actual = flushConfiguration3.IsEnabled();
    expected = false;
    ASSERT_EQ(expected, actual);

    // When: set flush as true and journal as true
    FlushConfiguration flushConfiguration4(&mockConfigManager);

    // Then: IsEnabled() has to return false
    actual = flushConfiguration4.IsEnabled();
    expected = false;
    ASSERT_EQ(expected, actual);

    // When: set config manager returns error
    FlushConfiguration flushConfiguration5(&mockConfigManager);

    // Then: IsEnabled() has to return false
    actual = flushConfiguration5.IsEnabled();
    expected = false;
    ASSERT_EQ(expected, actual);
}

} // namespace pos
