#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/replay/filter_logs.h"

namespace pos
{
class MockFilterLogs : public FilterLogs
{
public:
    using FilterLogs::FilterLogs;
    MOCK_METHOD(int, Start, (), (override));
    MOCK_METHOD(ReplayTaskId, GetId, (), (override));
    MOCK_METHOD(int, GetWeight, (), (override));
    MOCK_METHOD(int, GetNumSubTasks, (), (override));
};

} // namespace pos
