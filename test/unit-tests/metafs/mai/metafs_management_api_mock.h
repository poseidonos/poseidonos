#include <gmock/gmock.h>

#include <list>
#include <vector>

#include "src/metafs/mai/metafs_management_api.h"

namespace pos
{
class MockMetaFsManagementApi : public MetaFsManagementApi
{
public:
    using MetaFsManagementApi::MetaFsManagementApi;
    MOCK_METHOD(POS_EVENT_ID, InitializeSystem, (int arrayId, MetaStorageMediaInfoList* mediaInfoList));
    MOCK_METHOD(POS_EVENT_ID, CloseSystem, (int arrayId));
    MOCK_METHOD(uint64_t, GetEpochSignature, ());
    MOCK_METHOD(MetaFsStorageIoInfoList&, GetAllStoragePartitionInfo, ());
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, ());
    MOCK_METHOD(POS_EVENT_ID, LoadMbr, (bool& isNPOR));
    MOCK_METHOD(bool, CreateMbr, ());
    MOCK_METHOD(MetaStorageSubsystem*, GetMss, ());
    MOCK_METHOD(void, SetStatus, (bool isNormal));
};

} // namespace pos
