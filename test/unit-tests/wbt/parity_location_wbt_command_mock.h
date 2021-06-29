#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/parity_location_wbt_command.h"

namespace pos
{
class MockParityLocationWbtCommand : public ParityLocationWbtCommand
{
public:
    using ParityLocationWbtCommand::ParityLocationWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
