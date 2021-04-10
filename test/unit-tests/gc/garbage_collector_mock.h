#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/garbage_collector.h"

namespace pos
{
class MockGarbageCollector : public GarbageCollector
{
public:
    using GarbageCollector::GarbageCollector;
    MOCK_METHOD(int, Start, (), (override));
    MOCK_METHOD(void, End, (), (override));
    MOCK_METHOD(void, StateChanged, (StateContext* prev, StateContext* next), (override));
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
};

} // namespace pos
