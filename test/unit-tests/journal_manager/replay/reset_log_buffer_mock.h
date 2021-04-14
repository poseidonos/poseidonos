#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/reset_log_buffer.h"

namespace pos
{
class MockResetLogBuffer : public ResetLogBuffer
{
public:
    using ResetLogBuffer::ResetLogBuffer;
    MOCK_METHOD(int, Start, (), (override));
    MOCK_METHOD(ReplayTaskId, GetId, (), (override));
    MOCK_METHOD(int, GetWeight, (), (override));
    MOCK_METHOD(int, GetNumSubTasks, (), (override));
};

} // namespace pos
