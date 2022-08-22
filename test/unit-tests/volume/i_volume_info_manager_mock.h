#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/i_volume_info_manager.h"

namespace pos
{
class MockIVolumeInfoManager : public IVolumeInfoManager
{
public:
    using IVolumeInfoManager::IVolumeInfoManager;
    MOCK_METHOD(int, GetVolumeName, (int volId, std::string& volName), (override));
    MOCK_METHOD(int, GetVolumeID, (std::string volName), (override));
    MOCK_METHOD(int, GetVolumeCount, (), (override));
    MOCK_METHOD(int, CheckVolumeValidity, (int volId), (override));
    MOCK_METHOD(int, CheckVolumeValidity, (std::string name), (override));
    MOCK_METHOD(int, GetVolumeStatus, (int volId), (override));
    MOCK_METHOD(int, GetVolumeReplicationState, (int volId), (override));
    MOCK_METHOD(int, GetVolumeReplicationRoleProperty, (int volId), (override));
    MOCK_METHOD(uint64_t, EntireVolumeSize, (), (override));
    MOCK_METHOD(int, GetVolumeSize, (int volId, uint64_t& volSize), (override));
    MOCK_METHOD(VolumeList*, GetVolumeList, (), (override));
    MOCK_METHOD(std::string, GetStatusStr, (VolumeStatus status), (override));
    MOCK_METHOD(VolumeBase*, GetVolume, (int volId), (override));
    MOCK_METHOD(std::string, GetArrayName, (), (override));
    MOCK_METHOD(int, CancelVolumeReplay, (int volId), (override));
};

} // namespace pos
