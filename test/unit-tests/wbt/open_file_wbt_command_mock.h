#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/open_file_wbt_command.h"

namespace pos
{
class MockOpenFileWbtCommand : public OpenFileWbtCommand
{
public:
    using OpenFileWbtCommand::OpenFileWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
