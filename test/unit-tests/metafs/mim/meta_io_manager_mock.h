#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/meta_io_manager.h"

namespace pos
{
class MockMetaIoManager : public MetaIoManager
{
public:
    using MetaIoManager::MetaIoManager;
    MOCK_METHOD(const char*, GetModuleName, (), (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(bool, Bringup, (), (override));
    MOCK_METHOD(POS_EVENT_ID, ProcessNewReq, (MetaFsIoRequest & reqMsg), (override));
    MOCK_METHOD(bool, _IsSiblingModuleReady, (), (override));
};

} // namespace pos
