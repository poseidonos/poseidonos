#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_manager.h"

namespace pos
{
class MockVolumeManager : public VolumeManager
{
public:
    using VolumeManager::VolumeManager;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(int, Create, (std::string name, uint64_t size, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(int, Delete, (std::string name), (override));
    MOCK_METHOD(int, Mount, (std::string name, std::string subnqn), (override));
    MOCK_METHOD(int, Unmount, (std::string name), (override));
    MOCK_METHOD(int, UpdateQoS, (std::string name, uint64_t maxiops, uint64_t maxbw, uint64_t miniops, uint64_t minbw), (override));
    MOCK_METHOD(int, Rename, (std::string oldname, std::string newname), (override));
    MOCK_METHOD(void, DetachVolumes, (), (override));
    MOCK_METHOD(int, VolumeName, (int volId, std::string& volName), (override));
    MOCK_METHOD(int, VolumeID, (std::string volName), (override));
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
    MOCK_METHOD(void, StateChanged, (StateContext * prev, StateContext* next), (override));
    MOCK_METHOD(std::string, GetArrayName, (), (override));
};

} // namespace pos
