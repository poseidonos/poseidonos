#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_allocator_wbt.h"

namespace pos
{
class MockIAllocatorWbt : public IAllocatorWbt
{
public:
    using IAllocatorWbt::IAllocatorWbt;
    MOCK_METHOD(void, SetNormalGcThreshold, (uint32_t inputThreshold), (override));
    MOCK_METHOD(void, SetUrgentThreshold, (uint32_t inputThreshold), (override));
    MOCK_METHOD(int, GetMeta, (WBTAllocatorMetaType type, std::string fname, MetaFileIntf* file), (override));
    MOCK_METHOD(int, SetMeta, (WBTAllocatorMetaType type, std::string fname, MetaFileIntf* file), (override));
    MOCK_METHOD(int, GetBitmapLayout, (std::string fname), (override));
    MOCK_METHOD(int, GetInstantMetaInfo, (std::string fname), (override));
    MOCK_METHOD(void, FlushAllUserdataWBT, (), (override));
};

} // namespace pos
