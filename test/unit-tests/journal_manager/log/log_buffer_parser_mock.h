#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log/log_buffer_parser.h"

namespace pos
{
class MockLogBufferParser : public LogBufferParser
{
public:
    using LogBufferParser::LogBufferParser;
};

} // namespace pos
