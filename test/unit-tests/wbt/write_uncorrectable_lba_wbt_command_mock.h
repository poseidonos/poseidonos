#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/write_uncorrectable_lba_wbt_command.h"

namespace pos
{
class MockWriteUncorrectableLbaWbtCommand : public WriteUncorrectableLbaWbtCommand
{
public:
    using WriteUncorrectableLbaWbtCommand::WriteUncorrectableLbaWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
