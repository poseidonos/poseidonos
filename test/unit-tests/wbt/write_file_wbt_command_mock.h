#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/write_file_wbt_command.h"

namespace pos
{
class MockWriteFileWbtCommand : public WriteFileWbtCommand
{
public:
    using WriteFileWbtCommand::WriteFileWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
