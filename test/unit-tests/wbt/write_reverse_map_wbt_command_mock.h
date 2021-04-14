#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/write_reverse_map_wbt_command.h"

namespace pos
{
class MockWriteReverseMapWbtCommand : public WriteReverseMapWbtCommand
{
public:
    using WriteReverseMapWbtCommand::WriteReverseMapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
