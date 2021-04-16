#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log/block_write_done_log_handler.h"

namespace pos
{
class MockBlockWriteDoneLogHandler : public BlockWriteDoneLogHandler
{
public:
    using BlockWriteDoneLogHandler::BlockWriteDoneLogHandler;
    MOCK_METHOD(LogType, GetType, (), (override));
    MOCK_METHOD(uint32_t, GetSize, (), (override));
    MOCK_METHOD(char*, GetData, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(uint32_t, GetSeqNum, (), (override));
    MOCK_METHOD(void, SetSeqNum, (uint32_t num), (override));
};

} // namespace pos
