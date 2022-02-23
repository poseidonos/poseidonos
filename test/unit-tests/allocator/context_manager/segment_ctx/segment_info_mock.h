#include <gmock/gmock.h>

#include <list>
#include <string>
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
    MOCK_METHOD(int32_t, DecreaseValidBlockCount, (uint32_t dec), (override));
    MOCK_METHOD(void, SetOccupiedStripeCount, (uint32_t cnt), (override));
    MOCK_METHOD(uint32_t, GetOccupiedStripeCount, (), (override));
    MOCK_METHOD(uint32_t, IncreaseOccupiedStripeCount, (), (override));
};

} // namespace pos
