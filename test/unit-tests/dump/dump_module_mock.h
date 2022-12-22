#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/debug_lib/debug_info_queue.h"

namespace pos
{
template<typename T>
class MockDumpObject : public DumpObject<T>
{
public:
    using DumpObject::DumpObject;
};

class MockAbstractDumpModule : public DebugInfoQueueInstance
{
public:
    using DebugInfoQueueInstance::DebugInfoQueueInstance;
    MOCK_METHOD(void, SetEnable, (bool enable), (override));
    MOCK_METHOD(bool, IsEnable, (), (override));
    MOCK_METHOD(uint32_t, GetPoolSize, (), (override));
};

template<typename T>
class MockDumpModule : public DebugInfoQueue<T>
{
public:
    using DebugInfoQueue::DebugInfoQueue;
    MOCK_METHOD(void, SetEnable, (bool enable), (override));
    MOCK_METHOD(bool, IsEnable, (), (override));
    MOCK_METHOD(uint32_t, GetPoolSize, (), (override));
};

} // namespace pos
