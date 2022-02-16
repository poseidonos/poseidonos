#include <gmock/gmock.h>
#include <string>
#include <list>
#include <utility>
#include <vector>
#include "src/allocator/context_manager/segment_ctx/segment_info.h"

namespace pos
{
class MockSegmentInfo : public SegmentInfo
{
public:
    using SegmentInfo::SegmentInfo;
    MOCK_METHOD(uint32_t, GetValidBlockCount, (), (override));
    MOCK_METHOD(void, SetValidBlockCount, (int cnt), (override));
    MOCK_METHOD(uint32_t, IncreaseValidBlockCount, (uint32_t inc), (override));
    MOCK_METHOD((std::pair<bool, SegmentState>), DecreaseValidBlockCount, (uint32_t dec), (override));
    MOCK_METHOD(void, SetOccupiedStripeCount, (uint32_t cnt), (override));
    MOCK_METHOD(uint32_t, GetOccupiedStripeCount, (), (override));
    MOCK_METHOD(uint32_t, IncreaseOccupiedStripeCount, (), (override));
    MOCK_METHOD(SegmentState, GetState, (), (override));
    MOCK_METHOD(void, MoveToNvramState, (), (override));
    MOCK_METHOD(bool, MoveToSsdStateOrFreeStateIfItBecomesEmpty, (), (override));
    MOCK_METHOD(bool, MoveToVictimState, (), (override));
    MOCK_METHOD(uint32_t, GetValidBlockCountIfSsdState, (), (override));
};

} // namespace pos
