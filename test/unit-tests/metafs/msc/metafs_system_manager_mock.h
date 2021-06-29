#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/msc/metafs_system_manager.h"

namespace pos
{
class MockMetaFsSystemManager : public MetaFsSystemManager
{
public:
    using MetaFsSystemManager::MetaFsSystemManager;
    MOCK_METHOD(const char*, GetModuleName, (), (override));
    MOCK_METHOD(bool, Init, (std::string & arrayName, MetaStorageMediaInfoList & mediaInfoList), (override));
    MOCK_METHOD(bool, Bringup, (std::string & arrayName), (override));
    MOCK_METHOD(POS_EVENT_ID, ProcessNewReq, (MetaFsControlReqMsg & reqMsg), (override));
    MOCK_METHOD(bool, IsMounted, (std::string & arrayName), (override));
    MOCK_METHOD(bool, _IsSiblingModuleReady, (), (override));
};

} // namespace pos
