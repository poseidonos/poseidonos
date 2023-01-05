#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/debug_lib/dump_manager.h"

namespace pos
{
class MockDumpManager : public DumpManager
{
public:
    using DumpManager::DumpManager;
};

} // namespace pos
