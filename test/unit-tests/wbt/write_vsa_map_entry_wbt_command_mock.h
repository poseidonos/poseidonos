#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/write_vsa_map_entry_wbt_command.h"

namespace pos
{
class MockWriteVsaMapEntryWbtCommand : public WriteVsaMapEntryWbtCommand
{
public:
    using WriteVsaMapEntryWbtCommand::WriteVsaMapEntryWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
