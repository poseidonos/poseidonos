#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_partition_size_wbt_command.h"

namespace pos
{
class MockGetPartitionSizeWbtCommand : public GetPartitionSizeWbtCommand
{
public:
    using GetPartitionSizeWbtCommand::GetPartitionSizeWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
