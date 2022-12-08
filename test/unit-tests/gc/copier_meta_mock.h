#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/copier_meta.h"

namespace pos
{
class MockCopierMeta : public CopierMeta
{
public:
    using CopierMeta::CopierMeta;
    MOCK_METHOD(void, GetBuffers, (uint32_t count, std::vector<void*>* retBuffers), (override));
    MOCK_METHOD(void, ReturnBuffer, (StripeId stripeId, void* buffer), (override));
    MOCK_METHOD(void, SetStartCopyStripes, (), (override));
    MOCK_METHOD(void, SetStartCopyBlks, (uint32_t blocks), (override));
    MOCK_METHOD(void, SetDoneCopyBlks, (uint32_t blocks), (override));
    MOCK_METHOD(uint32_t, GetStartCopyBlks, (), (override));
    MOCK_METHOD(uint32_t, GetDoneCopyBlks, (), (override));
    MOCK_METHOD(void, InitProgressCount, (), (override));
    MOCK_METHOD(uint32_t, SetInUseBitmap, (), (override));
    MOCK_METHOD(bool, IsSynchronized, (), (override));
    MOCK_METHOD(bool, IsAllVictimSegmentCopyDone, (), (override));
    MOCK_METHOD(bool, IsCopyDone, (), (override));
    MOCK_METHOD(bool, IsReadytoCopy, (uint32_t index), (override));
    MOCK_METHOD(uint32_t, GetStripePerSegment, (), (override));
    MOCK_METHOD(uint32_t, GetBlksPerStripe, (), (override));
    MOCK_METHOD(VictimStripe*, GetVictimStripe, (uint32_t victimSegmentIndex, uint32_t stripeOffset), (override));
    MOCK_METHOD(GcStripeManager*, GetGcStripeManager, (), (override));
    MOCK_METHOD(std::string, GetArrayName, (), (override));
};

} // namespace pos
