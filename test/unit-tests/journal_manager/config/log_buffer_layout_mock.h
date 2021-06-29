#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/config/log_buffer_layout.h"

namespace pos
{
class MockLogGroupLayout : public LogGroupLayout
{
public:
    using LogGroupLayout::LogGroupLayout;
};

class MockLogBufferLayout : public LogBufferLayout
{
public:
    using LogBufferLayout::LogBufferLayout;
};

} // namespace pos
