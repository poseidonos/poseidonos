#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/do_gc_wbt_command.h"

namespace pos
{
class MockDoGcWbtCommand : public DoGcWbtCommand
{
public:
    using DoGcWbtCommand::DoGcWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
