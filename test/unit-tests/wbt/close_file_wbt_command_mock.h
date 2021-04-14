#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/close_file_wbt_command.h"

namespace pos
{
class MockCloseFileWbtCommand : public CloseFileWbtCommand
{
public:
    using CloseFileWbtCommand::CloseFileWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
