#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/replay/replay_log_list.h"

namespace pos
{
class MockReplayLogList : public ReplayLogList
{
public:
    using ReplayLogList::ReplayLogList;
    MOCK_METHOD(void, Init, (int numLogGroups), (override));
    MOCK_METHOD(void, AddLog, (int logGroupId, LogHandlerInterface* log), (override));
    MOCK_METHOD(void, SetLogGroupFooter, (int logGroupId, LogGroupFooter footer), (override));
    MOCK_METHOD(bool, IsEmpty, (), (override));
    MOCK_METHOD(LogGroupFooter, GetLogGroupFooter, (int logGroupId), (override));
    MOCK_METHOD(int, EraseReplayLogGroup, (int logGroupId, uint32_t seqNum), (override));
    MOCK_METHOD(void, SetSegInfoFlushed, (int logGroupId), (override));
    MOCK_METHOD(std::vector<ReplayLog>, PopReplayLogGroup, (), (override));
    MOCK_METHOD(void, PrintLogStatistics, (), (override));
};

} // namespace pos
