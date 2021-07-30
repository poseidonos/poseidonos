#include <gmock/gmock.h>

#include <list>
#include <vector>

#include "src/metafs/msc/metafs_system_manager.h"

namespace pos
{
class MockMetaFsSystemManager : public MetaFsSystemManager
{
public:
    using MetaFsSystemManager::MetaFsSystemManager;
    MOCK_METHOD(bool, Init, (int arrayId, MetaStorageMediaInfoList & mediaInfoList), (override));
    MOCK_METHOD(bool, Bringup, (int arrayId), (override));
    MOCK_METHOD(POS_EVENT_ID, ProcessNewReq, (MetaFsControlReqMsg & reqMsg), (override));
    MOCK_METHOD(bool, IsMounted, (int arrayId), (override));
    MOCK_METHOD(bool, _IsSiblingModuleReady, (), (override));
};

} // namespace pos
