#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/create_file_system_wbt_command.h"

namespace pos
{
class MockCreateFileSystemWbtCommand : public CreateFileSystemWbtCommand
{
public:
    using CreateFileSystemWbtCommand::CreateFileSystemWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
