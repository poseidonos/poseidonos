#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mpio_state_execute_entry.h"

namespace pos
{
class MockMpioStateExecuteEntry : public MpioStateExecuteEntry
{
public:
    using MpioStateExecuteEntry::MpioStateExecuteEntry;
};

} // namespace pos
