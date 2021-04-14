#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/read_vsa_map_entry_wbt_command.h"

namespace pos
{
class MockReadVsaMapEntryWbtCommand : public ReadVsaMapEntryWbtCommand
{
public:
    using ReadVsaMapEntryWbtCommand::ReadVsaMapEntryWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
