#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/mss_disk_place.h"

namespace pos
{
class MockMssDiskPlace : public MssDiskPlace
{
public:
    using MssDiskPlace::MssDiskPlace;
    MOCK_METHOD(const pos::PartitionLogicalSize*, GetPartitionSizeInfo, (), (override));
    MOCK_METHOD(uint64_t, GetMetaDiskCapacity, (), (override));
    MOCK_METHOD(pos::LogicalBlkAddr, CalculateOnDiskAddress, (uint64_t metaLpn), (override));
    MOCK_METHOD(uint32_t, GetMaxLpnCntPerIOSubmit, (), (override));
};

} // namespace pos
