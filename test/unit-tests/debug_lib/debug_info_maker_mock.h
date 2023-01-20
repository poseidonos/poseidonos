#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/debug_lib/debug_info_maker.h"

namespace pos
{
class MockDebugInfoInstance : public DebugInfoInstance
{
public:
    using DebugInfoInstance::DebugInfoInstance;
    MOCK_METHOD(void, DeRegisterDebugInfoInstance, (std::string str), (override));
};

template <typename T>
class MockDebugInfoMaker : public DebugInfoMaker<T>
{
public:
    using DebugInfoMaker::DebugInfoMaker;
    MOCK_METHOD(void, MakeDebugInfo, (T& obj), (override));
    MOCK_METHOD(void, AddDebugInfo, (uint64_t userSpecific), (override));
    MOCK_METHOD(void, RegisterDebugInfo, (std::string name, uint32_t entryCount, bool asyncLogging, uint64_t inputTimerUsec, bool enabled), (override));
    MOCK_METHOD(void, DeRegisterDebugInfo, (std::string name), (override));
};

} // namespace pos
