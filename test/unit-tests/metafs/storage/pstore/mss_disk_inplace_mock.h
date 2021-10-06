#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/mss_disk_inplace.h"

namespace pos
{
class MockMssDiskInplace : public MssDiskInplace
{
public:
    using MssDiskInplace::MssDiskInplace;
    MOCK_METHOD(uint32_t, GetMaxLpnCntPerIOSubmit, (), (override));
    MOCK_METHOD(LogicalBlkAddr, CalculateOnDiskAddress, (uint64_t metaLpn));
};

} // namespace pos
