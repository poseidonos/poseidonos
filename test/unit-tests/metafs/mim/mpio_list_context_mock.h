#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mpio_list_context.h"

namespace pos
{
class MockMpioListContext : public MpioListContext
{
public:
    using MpioListContext::MpioListContext;
};

} // namespace pos
