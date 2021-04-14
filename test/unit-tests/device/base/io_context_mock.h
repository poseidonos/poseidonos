#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/base/io_context.h"

namespace pos
{
class MockIOContext : public IOContext
{
public:
    using IOContext::IOContext;
};

} // namespace pos
