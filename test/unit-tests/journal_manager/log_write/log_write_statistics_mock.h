#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_write/log_write_statistics.h"

namespace pos
{
class MockLogWriteStatistics : public LogWriteStatistics
{
public:
    using LogWriteStatistics::LogWriteStatistics;
    MOCK_METHOD(void, Init, (int numLogGroups), (override));
    MOCK_METHOD(bool, UpdateStatus, (LogWriteContext* context), (override));
    MOCK_METHOD(void, AddToList, (LogWriteContext* context), (override));
    MOCK_METHOD(void, PrintStats, (int groupId), (override));
};

} // namespace pos
