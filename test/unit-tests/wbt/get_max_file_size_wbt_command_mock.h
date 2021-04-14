#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_max_file_size_wbt_command.h"

namespace pos
{
class MockGetMaxFileSizeWbtCommand : public GetMaxFileSizeWbtCommand
{
public:
    using GetMaxFileSizeWbtCommand::GetMaxFileSizeWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
