#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/dump_disk_layout_wbt_command.h"

namespace pos
{
class MockDumpDiskLayoutWbtCommand : public DumpDiskLayoutWbtCommand
{
public:
    using DumpDiskLayoutWbtCommand::DumpDiskLayoutWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
