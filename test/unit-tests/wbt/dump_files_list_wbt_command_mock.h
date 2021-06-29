#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/dump_files_list_wbt_command.h"

namespace pos
{
class MockDumpFilesListWbtCommand : public DumpFilesListWbtCommand
{
public:
    using DumpFilesListWbtCommand::DumpFilesListWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
