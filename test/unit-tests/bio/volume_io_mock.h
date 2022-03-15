#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/bio/volume_io.h"

namespace pos
{
class MockRbaAndSize : public RbaAndSize
{
public:
    using RbaAndSize::RbaAndSize;
};

class MockVolumeIo : public VolumeIo
{
public:
    using VolumeIo::VolumeIo;
    MockVolumeIo(void)
    : VolumeIo(nullptr, 0, 0)
    {
    }
    MOCK_METHOD(VolumeIoSmartPtr, Split, (uint32_t sectors, bool removalFromTail), (override));
    MOCK_METHOD(VolumeIoSmartPtr, GetOriginVolumeIo, (), (override));
    MOCK_METHOD(uint32_t, GetVolumeId, (), (override));
    MOCK_METHOD(uint32_t, GetOriginCore, (), (override));
    MOCK_METHOD(uint64_t, GetSize, (), (override));
    MOCK_METHOD(void, SetLsidEntry, (StripeAddr& lsidEntry), (override));
    MOCK_METHOD(const StripeAddr&, GetLsidEntry, (), (override));
    MOCK_METHOD(StripeId, GetWbLsid, (), (override));
    MOCK_METHOD(const StripeAddr&, GetOldLsidEntry, (), (override));
    MOCK_METHOD(const VirtualBlkAddr&, GetVsa, (), (override));
    MOCK_METHOD(uint64_t, GetSectorRba, (), (override));
    MOCK_METHOD(bool, _IsInvalidLsidEntry, (StripeAddr& inputLsidEntry), (override));
    MOCK_METHOD(int, GetArrayId, (), (override));
};

} // namespace pos
