#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/debug_lib/debug_info_queue.h"

namespace pos
{
template <typename T>
class MockDumpObject : public DumpObject<T>
{
public:
    using DumpObject::DumpObject;
};

class MockDebugInfoQueueInstance : public DebugInfoQueueInstance
{
public:
    using DebugInfoQueueInstance::DebugInfoQueueInstance;
    MOCK_METHOD(void, SetEnable, (bool enable), (override));
    MOCK_METHOD(bool, IsEnable, (), (override));
    MOCK_METHOD(uint64_t, GetPoolSize, (), (override));
};

template <typename T>
class MockDebugInfoQueue : public DebugInfoQueue<T>
{
public:
    using DebugInfoQueue::DebugInfoQueue;
    MOCK_METHOD(void, SetEnable, (bool enable), (override));
    MOCK_METHOD(bool, IsEnable, (), (override));
    MOCK_METHOD(uint64_t, GetPoolSize, (), (override));
};

} // namespace pos
