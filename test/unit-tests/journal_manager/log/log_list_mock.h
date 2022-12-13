#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log/log_list.h"

namespace pos
{
class MockLogList : public LogList
{
public:
    using LogList::LogList;
    MOCK_METHOD(void, AddLog, (int logGroupId, LogHandlerInterface* log), (override));
    MOCK_METHOD(void, SetLogGroupFooter, (int logGroupId, LogGroupFooter footer), (override));
    MOCK_METHOD(bool, IsEmpty, (), (override));
};

} // namespace pos
