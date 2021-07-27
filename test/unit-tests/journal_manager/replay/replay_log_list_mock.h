#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_log_list.h"

namespace pos
{
class MockReplayLogList : public ReplayLogList
{
public:
    using ReplayLogList::ReplayLogList;
    MOCK_METHOD(void, AddLog, (LogHandlerInterface* log), (override));
    MOCK_METHOD(bool, IsEmpty, (), (override));
    MOCK_METHOD(void, SetLogGroupFooter, (uint64_t seqNum, LogGroupFooter footer), (override));
};

} // namespace pos
