#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/read_vsa_map_wbt_command.h"

namespace pos
{
class MockReadVsaMapWbtCommand : public ReadVsaMapWbtCommand
{
public:
    using ReadVsaMapWbtCommand::ReadVsaMapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
