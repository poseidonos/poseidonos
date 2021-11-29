#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/network/nvmf_target.h"

namespace pos
{
class MockNvmfTarget : public NvmfTarget
{
public:
    using NvmfTarget::NvmfTarget;
    MOCK_METHOD(bool, CreatePosBdev, (const string& bdevName, const string uuid, uint32_t id,
        uint64_t volumeSizeInMb, uint32_t blockSize, bool volumeTypeInMem, const string& arrayName,
        uint64_t arrayId), (override));
    MOCK_METHOD(bool, DeletePosBdev, (const string& bdevName), (override));
    MOCK_METHOD(bool, DeletePosBdevAll, (string bdevName, uint64_t time), (override));
    MOCK_METHOD(bool, DetachNamespace, (const string& nqn, uint32_t nsid, PosNvmfEventDoneCallback_t cb, void* cbArg), (override));
    MOCK_METHOD(bool, DetachNamespaceAll, (const string& nqn, PosNvmfEventDoneCallback_t cb, void* cbArg), (override));
    MOCK_METHOD(struct spdk_nvmf_ns*, GetNamespace, (struct spdk_nvmf_subsystem * subsystem, const string& bdevName), (override));
    MOCK_METHOD(void, SetVolumeQos, (const string& bdevName, uint64_t maxIops, uint64_t maxBw), (override));
    MOCK_METHOD(string, GetBdevName, (uint32_t id, string arrayName), (override));
    MOCK_METHOD(int32_t, GetVolumeNqnId, (const string& subnqn), (override));
    MOCK_METHOD(spdk_nvmf_subsystem*, FindSubsystem, (const string& subnqn), (override));
    MOCK_METHOD(bool, TryToAttachNamespace, (const string& nqn, int volId, string& arrayName, uint64_t time), (override));
    MOCK_METHOD(bool, CheckSubsystemExistance, (), (override));
    MOCK_METHOD(bool, CheckVolumeAttached, (int volId, string arrayName), (override));
    MOCK_METHOD(bool, SetSubsystemArrayName, (string & subnqn, string& arrayName), (override));
    MOCK_METHOD(string, GetSubsystemArrayName, (string & subnqn), (override));
    MOCK_METHOD(void, RemoveSubsystemArrayName, (string & subnqn), (override));
};

} // namespace pos
