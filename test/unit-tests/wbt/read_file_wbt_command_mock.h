#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/read_file_wbt_command.h"

namespace pos
{
class MockReadFileWbtCommand : public ReadFileWbtCommand
{
public:
    using ReadFileWbtCommand::ReadFileWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
