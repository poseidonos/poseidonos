#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/debug_lib/dump_shared_ptr.h"

namespace pos
{
template<typename T>
class MockDumpSharedPtr : public DumpSharedPtr<T>
{
public:
    using DumpSharedPtr::DumpSharedPtr;
};

template<typename T>
class MockDumpSharedModule : public DumpSharedModule<T>
{
public:
    using DumpSharedModule::DumpSharedModule;
};

class MockDumpSharedModuleInstanceEnable : public DumpSharedModuleInstanceEnable
{
public:
    using DumpSharedModuleInstanceEnable::DumpSharedModuleInstanceEnable;
};

template<typename T, typename moduleNumber>
class MockDumpSharedModuleInstance : public DumpSharedModuleInstance<T, moduleNumber>
{
public:
    using DumpSharedModuleInstance::DumpSharedModuleInstance;
};

} // namespace pos
