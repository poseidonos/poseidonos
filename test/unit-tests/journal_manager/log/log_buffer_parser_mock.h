#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log/log_buffer_parser.h"

namespace pos
{
class MockLogBufferParser : public LogBufferParser
{
public:
    using LogBufferParser::LogBufferParser;
    MOCK_METHOD(int, GetLogs, (void* buffer, int logGroupId, uint64_t bufferSize, LogList& logs), (override));
};

} // namespace pos
