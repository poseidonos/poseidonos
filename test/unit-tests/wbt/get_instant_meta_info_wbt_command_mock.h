#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_instant_meta_info_wbt_command.h"

namespace pos
{
class MockGetInstantMetaInfoWbtCommand : public GetInstantMetaInfoWbtCommand
{
public:
    using GetInstantMetaInfoWbtCommand::GetInstantMetaInfoWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
