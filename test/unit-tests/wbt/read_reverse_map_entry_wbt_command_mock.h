#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/read_reverse_map_entry_wbt_command.h"

namespace pos
{
class MockReadReverseMapEntryWbtCommand : public ReadReverseMapEntryWbtCommand
{
public:
    using ReadReverseMapEntryWbtCommand::ReadReverseMapEntryWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
