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
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
    MOCK_METHOD(void, Pause, (), (override));
    MOCK_METHOD(void, Resume, (), (override));
    MOCK_METHOD(bool, IsPaused, (), (override));
    MOCK_METHOD(int, DisableThresholdCheck, (), (override));
    MOCK_METHOD(int, IsEnabled, (), (override));
    MOCK_METHOD(bool, GetGcRunning, (), (override));
    MOCK_METHOD(struct timeval, GetStartTime, (), (override));
    MOCK_METHOD(struct timeval, GetEndTime, (), (override));
};

} // namespace pos
