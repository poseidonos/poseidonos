#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_event.h"

namespace pos
{
class MockReplayEvent : public ReplayEvent
{
public:
    using ReplayEvent::ReplayEvent;
    MOCK_METHOD(int, Replay, (), (override));
    MOCK_METHOD(ReplayEventType, GetType, (), (override));
};

class MockReplayBlockMapUpdate : public ReplayBlockMapUpdate
{
public:
    using ReplayBlockMapUpdate::ReplayBlockMapUpdate;
    MOCK_METHOD(int, Replay, (), (override));
};

class MockReplayStripeMapUpdate : public ReplayStripeMapUpdate
{
public:
    using ReplayStripeMapUpdate::ReplayStripeMapUpdate;
    MOCK_METHOD(int, Replay, (), (override));
};

class MockReplayStripeAllocation : public ReplayStripeAllocation
{
public:
    using ReplayStripeAllocation::ReplayStripeAllocation;
    MOCK_METHOD(int, Replay, (), (override));
};

class MockReplaySegmentAllocation : public ReplaySegmentAllocation
{
public:
    using ReplaySegmentAllocation::ReplaySegmentAllocation;
    MOCK_METHOD(int, Replay, (), (override));
};

class MockReplayStripeFlush : public ReplayStripeFlush
{
public:
    using ReplayStripeFlush::ReplayStripeFlush;
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos
