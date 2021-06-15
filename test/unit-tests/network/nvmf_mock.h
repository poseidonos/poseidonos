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
    MOCK_METHOD(bool, VolumeCreated, (string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, string arrayName, int arrayId), (override));
    MOCK_METHOD(bool, VolumeDeleted, (string volName, int volID, uint64_t volSizeByte, string arrayName, int arrayId), (override));
    MOCK_METHOD(bool, VolumeMounted, (string volName, string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, string arrayName, int arrayId), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (string volName, int volID, string arrayName, int arrayId), (override));
    MOCK_METHOD(bool, VolumeLoaded, (string volName, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, string arrayName, int arrayId), (override));
    MOCK_METHOD(bool, VolumeUpdated, (string volName, int volID, uint64_t maxiops, uint64_t maxbw, string arrayName, int arrayId), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList, string arrayName, int arrayId), (override));
};

} // namespace pos
