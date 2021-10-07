#include "src/cpu_affinity/affinity_config_parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/master_context/config_manager_mock.h"

using namespace std;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(AffinityConfigParser, AffinityConfigParser_Stack_Success)
{
    // Given
    MockConfigManager mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault([&mockConfigManager](string module, string key, void* value, ConfigType type) {
        if (key == "use_config")
        {
            *(static_cast<bool*>(value)) = true;
        }
        return 0;
    });

    // When : Create AffinityConfigParser in stack
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).Times(AtLeast(2));
    AffinityConfigParser affinityConfigParser(mockConfigManager);

    // Then :  Do nothing
}

TEST(AffinityConfigParser, AffinityConfigParser_Stack_NotUseConfig)
{
    // Given
    MockConfigManager mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(1));

    // When : Create AffinityConfigParser in stack
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).Times(1);
    AffinityConfigParser affinityConfigParser(mockConfigManager);

    // Then :  Do nothing
}

TEST(AffinityConfigParser, AffinityConfigParser_Stack_ConfigFail)
{
    // Given
    MockConfigManager mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault([&mockConfigManager](string module, string key, void* value, ConfigType type)
    {
        if (key == "use_config")
        {
            *(static_cast<bool*>(value)) = true;
            return 0;
        }
        else
        {
            return 1;
        }
    });

    // When : Create AffinityConfigParser in stack
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).Times(AtLeast(2));
    AffinityConfigParser affinityConfigParser(mockConfigManager);

    // Then :  Do nothing
}
TEST(AffinityConfigParser, AffinityConfigParser_Heap)
{
    // Given
    MockConfigManager mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(1));

    // When : Create AffinityConfigParser in heap
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).Times(1);
    AffinityConfigParser* affinityConfigParser = new AffinityConfigParser(mockConfigManager);

    // Then :  Release memory
    delete affinityConfigParser;
}

TEST(AffinityConfigParser, GetDescriptions_ReturnValue)
{
    // Given
    NiceMock<MockConfigManager> mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(1));

    // When : Call GetDescription
    AffinityConfigParser affinityConfigParser(mockConfigManager);
    CoreDescriptionArray coreDescriptionArray = affinityConfigParser.GetDescriptions();

    // Then :  Do nothing
}

TEST(AffinityConfigParser, IsStringDescripted_ReturnValue)
{
    // Given
    NiceMock<MockConfigManager> mockConfigManager;
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(1));

    // When : Call IsStringDescripted
    AffinityConfigParser affinityConfigParser(mockConfigManager);
    bool result = affinityConfigParser.IsStringDescripted();

    // Then :  Check return value
    ASSERT_EQ(true, result);
}

} // namespace pos
