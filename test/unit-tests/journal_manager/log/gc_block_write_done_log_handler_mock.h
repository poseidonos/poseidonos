#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log/gc_block_write_done_log_handler.h"

namespace pos
{
class MockGcBlockWriteDoneLogHandler : public GcBlockWriteDoneLogHandler
{
public:
    using GcBlockWriteDoneLogHandler::GcBlockWriteDoneLogHandler;
    MOCK_METHOD(LogType, GetType, (), (override));
    MOCK_METHOD(uint32_t, GetSize, (), (override));
    MOCK_METHOD(char*, GetData, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(uint32_t, GetSeqNum, (), (override));
    MOCK_METHOD(void, SetSeqNum, (uint32_t num), (override));
};

} // namespace pos
