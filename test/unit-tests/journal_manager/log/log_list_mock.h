#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log/log_list.h"

namespace pos
{
class MockLogList : public LogList
{
public:
    using LogList::LogList;
    MOCK_METHOD(void, AddLog, (LogHandlerInterface * log), (override));
    MOCK_METHOD(bool, IsEmpty, (), (override));
    MOCK_METHOD(void, SetLogGroupFooter, (uint32_t seqNum, LogGroupFooter footer), (override));
};

} // namespace pos
