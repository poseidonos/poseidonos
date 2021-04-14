#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/gc_wbt_command.h"

namespace pos
{
class MockGcWbtCommand : public GcWbtCommand
{
public:
    using GcWbtCommand::GcWbtCommand;
};

} // namespace pos
