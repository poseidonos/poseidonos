#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/write_vsa_map_wbt_command.h"

namespace pos
{
class MockWriteVsaMapWbtCommand : public WriteVsaMapWbtCommand
{
public:
    using WriteVsaMapWbtCommand::WriteVsaMapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
