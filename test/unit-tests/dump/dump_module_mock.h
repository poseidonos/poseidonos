#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/dump/dump_module.h"

namespace pos
{
template<typename T>
class MockDumpObject : public DumpObject<T>
{
public:
    using DumpObject::DumpObject;
};

class MockAbstractDumpModule : public AbstractDumpModule
{
public:
    using AbstractDumpModule::AbstractDumpModule;
    MOCK_METHOD(void, SetEnable, (bool enable), (override));
    MOCK_METHOD(bool, IsEnable, (), (override));
    MOCK_METHOD(uint32_t, GetPoolSize, (), (override));
};

template<typename T>
class MockDumpModule : public DumpModule<T>
{
public:
    using DumpModule::DumpModule;
    MOCK_METHOD(void, SetEnable, (bool enable), (override));
    MOCK_METHOD(bool, IsEnable, (), (override));
    MOCK_METHOD(uint32_t, GetPoolSize, (), (override));
};

} // namespace pos
