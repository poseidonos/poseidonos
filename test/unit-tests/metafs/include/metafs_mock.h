#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/metafs.h"

namespace pos
{
class MockMetaFs : public MetaFs
{
public:
    using MetaFs::MetaFs;
    MOCK_METHOD(uint64_t, GetEpochSignature, (), (override));
    MOCK_METHOD(StripeId, GetTheLastValidStripeId, ());
    MOCK_METHOD(int, EstimateAlignedFileIOSize, (MetaFilePropertySet& prop, MetaVolumeType volumeType), (override));
    MOCK_METHOD(size_t, GetAvailableSpace, (MetaFilePropertySet& prop, MetaVolumeType volumeType), (override));
    MOCK_METHOD(MetaFsManagementApi*, GetMgmtApi, (), ());
    MOCK_METHOD(MetaFsFileControlApi*, GetCtrlApi, (), ());
    MOCK_METHOD(MetaFsIoApi*, GetIoApi, (), ());
    MOCK_METHOD(MetaFsWBTApi*, GetWbtApi, (), ());
};

} // namespace pos
