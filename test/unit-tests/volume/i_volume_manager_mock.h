#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/i_volume_manager.h"

namespace pos
{
class MockIVolumeManager : public IVolumeManager
{
public:
    using IVolumeManager::IVolumeManager;
    MOCK_METHOD(int, Create, (std::string name, uint64_t size, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(int, Delete, (std::string name), (override));
    MOCK_METHOD(int, Mount, (std::string name, std::string subnqn), (override));
    MOCK_METHOD(int, Unmount, (std::string name), (override));
    MOCK_METHOD(int, UpdateQoS, (std::string name, uint64_t maxiops, uint64_t maxbw, uint64_t miniops, uint64_t minbw), (override));
    MOCK_METHOD(int, Rename, (std::string oldname, std::string newname), (override));
    MOCK_METHOD(void, DetachVolumes, (), (override));
    MOCK_METHOD(int, GetVolumeName, (int volId, std::string& volName), (override));
    MOCK_METHOD(int, GetVolumeID, (std::string volName), (override));
    MOCK_METHOD(int, GetVolumeCount, (), (override));
    MOCK_METHOD(int, CheckVolumeValidity, (int volId), (override));
    MOCK_METHOD(int, GetVolumeStatus, (int volId), (override));
    MOCK_METHOD(uint64_t, EntireVolumeSize, (), (override));
    MOCK_METHOD(int, GetVolumeSize, (int volId, uint64_t& volSize), (override));
    MOCK_METHOD(VolumeList*, GetVolumeList, (), (override));
    MOCK_METHOD(std::string, GetStatusStr, (VolumeStatus status), (override));
    MOCK_METHOD(int, IncreasePendingIOCountIfNotZero, (int volId, VolumeStatus mounted, uint32_t ioCountToSubmit), (override));
    MOCK_METHOD(int, DecreasePendingIOCount, (int volId, VolumeStatus mounted, uint32_t ioCountCompleted), (override));
    MOCK_METHOD(VolumeBase*, GetVolume, (int volId), (override));
    MOCK_METHOD(std::string, GetArrayName, (), (override));
    MOCK_METHOD(int, CancelVolumeReplay, (int volId), (override));
};

} // namespace pos
