#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/write_whole_reverse_map_wbt_command.h"

namespace pos
{
class MockWriteWholeReverseMapWbtCommand : public WriteWholeReverseMapWbtCommand
{
public:
    using WriteWholeReverseMapWbtCommand::WriteWholeReverseMapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
