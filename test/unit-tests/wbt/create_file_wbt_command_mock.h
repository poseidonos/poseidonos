#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/create_file_wbt_command.h"

namespace pos
{
class MockCreateFileWbtCommand : public CreateFileWbtCommand
{
public:
    using CreateFileWbtCommand::CreateFileWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
