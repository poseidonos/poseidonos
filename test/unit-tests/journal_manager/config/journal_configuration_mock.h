#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/config/journal_configuration.h"

namespace pos
{
class MockJournalConfiguration : public JournalConfiguration
{
public:
    using JournalConfiguration::JournalConfiguration;
    MOCK_METHOD(int, Init, (uint64_t logBufferSize), (override));
    MOCK_METHOD(bool, IsEnabled, (), (override));
    MOCK_METHOD(bool, IsDebugEnabled, (), (override));
    MOCK_METHOD(int, GetNumLogGroups, (), (override));
    MOCK_METHOD(uint64_t, GetLogBufferSize, (), (override));
    MOCK_METHOD(uint64_t, GetLogGroupSize, (), (override));
    MOCK_METHOD(uint64_t, GetMetaPageSize, (), (override));
};

} // namespace pos
