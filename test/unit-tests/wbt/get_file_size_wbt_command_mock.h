#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_file_size_wbt_command.h"

namespace pos
{
class MockGetFileSizeWbtCommand : public GetFileSizeWbtCommand
{
public:
    using GetFileSizeWbtCommand::GetFileSizeWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
