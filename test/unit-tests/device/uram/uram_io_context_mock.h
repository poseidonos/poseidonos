#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/uram/uram_io_context.h"

namespace pos
{
class MockUramIOContext : public UramIOContext
{
public:
    using UramIOContext::UramIOContext;
};

} // namespace pos
