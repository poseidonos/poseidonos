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
    MOCK_METHOD(void, AllocateAndInitSegmentInfoData, (SegmentInfoData* segmentInfoData), (override));
    MOCK_METHOD(int, GetValidBlockCount, (), (override));
    MOCK_METHOD(void, SetValidBlockCount, (int cnt), (override));
    MOCK_METHOD(int, IncreaseValidBlockCount, (int inc), (override));
    MOCK_METHOD((std::pair<bool, SegmentState>), DecreaseValidBlockCount, (int dec, bool allowVictimSegRelease), (override));
    MOCK_METHOD(void, SetOccupiedStripeCount, (int cnt), (override));
    MOCK_METHOD(int, GetOccupiedStripeCount, (), (override));
    MOCK_METHOD(int, IncreaseOccupiedStripeCount, (), (override));
    MOCK_METHOD(SegmentState, GetState, (), (override));
    MOCK_METHOD(void, MoveToNvramState, (), (override));
    MOCK_METHOD(bool, MoveToSsdStateOrFreeStateIfItBecomesEmpty, (), (override));
    MOCK_METHOD(bool, MoveToVictimState, (), (override));
    MOCK_METHOD(int, GetValidBlockCountIfSsdState, (), (override));
};

} // namespace pos
