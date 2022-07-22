#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/config/journal_configuration.h"

namespace pos
{
class MockJournalConfiguration : public JournalConfiguration
{
public:
    using JournalConfiguration::JournalConfiguration;
    MOCK_METHOD(void, Init, (bool isWriteThroughEnabled), (override));
    MOCK_METHOD(int, SetLogBufferSize, (uint64_t loadedLogBufferSize, MetaFsFileControlApi* metaFsCtrl), (override));
    MOCK_METHOD(bool, IsEnabled, (), (override));
    MOCK_METHOD(bool, IsDebugEnabled, (), (override));
    MOCK_METHOD(bool, AreReplayWbStripesInUserArea, (), (override));
    MOCK_METHOD(bool, IsRocksdbEnabled, (), (override));
    MOCK_METHOD(int, GetNumLogGroups, (), (override));
    MOCK_METHOD(uint64_t, GetLogBufferSize, (), (override));
    MOCK_METHOD(uint64_t, GetLogGroupSize, (), (override));
    MOCK_METHOD(uint64_t, GetMetaPageSize, (), (override));
    MOCK_METHOD(MetaVolumeType, GetMetaVolumeToUse, (), (override));
    MOCK_METHOD(LogGroupLayout, GetLogBufferLayout, (int groupId), (override));
    MOCK_METHOD(std::string, GetRocksdbPath, (), (override));
};

} // namespace pos
