#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/read_whole_reverse_map_wbt_command.h"

namespace pos
{
class MockReadWholeReverseMapWbtCommand : public ReadWholeReverseMapWbtCommand
{
public:
    using ReadWholeReverseMapWbtCommand::ReadWholeReverseMapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
