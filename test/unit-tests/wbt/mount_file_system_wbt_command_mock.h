#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/mount_file_system_wbt_command.h"

namespace pos
{
class MockMountFileSystemWbtCommand : public MountFileSystemWbtCommand
{
public:
    using MountFileSystemWbtCommand::MountFileSystemWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
