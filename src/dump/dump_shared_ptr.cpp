#include "dump_shared_ptr.h"

#include <atomic>
namespace ibofos
{
void* gDumpSharedModulePtr[static_cast<int>(DumpSharedPtrType::MAX_DUMP_PTR)];
bool ibofos::DumpSharedModuleInstanceEnable::debugLevelEnable = false;

} // namespace ibofos
