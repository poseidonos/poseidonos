#include <gmock/gmock.h>

#include <list>
#include <string>
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
