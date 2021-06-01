#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mai/metafs_management_api.h"

namespace pos
{
class MockMetaFsManagementApi : public MetaFsManagementApi
{
public:
    using MetaFsManagementApi::MetaFsManagementApi;
    MOCK_METHOD(POS_EVENT_ID, InitializeSystem, (std::string & arrayName, MetaStorageMediaInfoList* mediaInfoList), (override));
    MOCK_METHOD(POS_EVENT_ID, CloseSystem, (std::string & arrayName), (override));
    MOCK_METHOD(uint64_t, GetEpochSignature, (), (override));
    MOCK_METHOD(MetaFsStorageIoInfoList&, GetAllStoragePartitionInfo, (), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(POS_EVENT_ID, LoadMbr, (bool& isNPOR), (override));
    MOCK_METHOD(bool, CreateMbr, (), (override));
    MOCK_METHOD(MetaStorageSubsystem*, GetMss, (), (override));
    MOCK_METHOD(void, SetStatus, (bool isNormal), (override));
};

} // namespace pos
