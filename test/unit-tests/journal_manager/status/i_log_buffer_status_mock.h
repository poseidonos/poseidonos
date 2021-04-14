#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/status/i_log_buffer_status.h"

namespace pos
{
class MockILogBufferStatus : public ILogBufferStatus
{
public:
    using ILogBufferStatus::ILogBufferStatus;
    MOCK_METHOD(LogGroupStatus, GetBufferStatus, (int logGroupId), (override));
    MOCK_METHOD(uint32_t, GetSequenceNumber, (int logGroupId), (override));
};

} // namespace pos
