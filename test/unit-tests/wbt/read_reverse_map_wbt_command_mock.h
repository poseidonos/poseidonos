#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/read_reverse_map_wbt_command.h"

namespace pos
{
class MockReadReverseMapWbtCommand : public ReadReverseMapWbtCommand
{
public:
    using ReadReverseMapWbtCommand::ReadReverseMapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
