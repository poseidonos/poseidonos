#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_gc_status_wbt_command.h"

namespace pos
{
class MockGetGcStatusWbtCommand : public GetGcStatusWbtCommand
{
public:
    using GetGcStatusWbtCommand::GetGcStatusWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
