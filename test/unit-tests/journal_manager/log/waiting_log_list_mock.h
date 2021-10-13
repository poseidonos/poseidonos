#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log/waiting_log_list.h"

namespace pos
{
class MockWaitingLogList : public WaitingLogList
{
public:
    using WaitingLogList::WaitingLogList;
    MOCK_METHOD(void, AddToList, (LogWriteContext * context), (override));
    MOCK_METHOD(LogWriteContext*, GetWaitingIo, (), (override));
    MOCK_METHOD(bool, AddToListIfNotEmpty, (LogWriteContext * context), (override));
};

} // namespace pos
