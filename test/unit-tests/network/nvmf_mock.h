#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/network/nvmf.h"

namespace pos
{
class MockNvmf : public Nvmf
{
public:
    using Nvmf::Nvmf;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
    MOCK_METHOD(bool, VolumeCreated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeLoaded, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeUpdated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeMounted, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeDeleted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
};

} // namespace pos
