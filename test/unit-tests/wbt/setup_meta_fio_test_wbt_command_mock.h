#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/setup_meta_fio_test_wbt_command.h"

namespace pos
{
class MockSetupMetaFioTestWbtCommand : public SetupMetaFioTestWbtCommand
{
public:
    using SetupMetaFioTestWbtCommand::SetupMetaFioTestWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
