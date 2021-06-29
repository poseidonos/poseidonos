#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_manager.h"

namespace pos
{
class MockQosManager : public QosManager
{
public:
    using QosManager::QosManager;
};

class MockQosVolumeManager : public QosVolumeManager
{
public:
    using QosVolumeManager::QosVolumeManager;
    MOCK_METHOD(bool, VolumeCreated, (std::string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeDeleted, (std::string volName, int volID, uint64_t volSizeByte), (override));
    MOCK_METHOD(bool, VolumeMounted, (std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (std::string volName, int volID), (override));
    MOCK_METHOD(bool, VolumeLoaded, (std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeUpdated, (std::string volName, int volID, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList), (override));
};

class MockQosEventManager : public QosEventManager
{
public:
    using QosEventManager::QosEventManager;
};

class MockQosSpdkManager : public QosSpdkManager
{
public:
    using QosSpdkManager::QosSpdkManager;
};

} // namespace pos
