#include "src/volume/volume_manager.h"

#include "src/include/array_mgmt_policy.h"
#include "src/array_mgmt/array_manager.h"
#include "src/qos/qos_manager.h"
#include "src/volume/volume.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"
#include "src/volume/volume_service.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(VolumeManager, VolumeManager_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    delete volumeManager;
}

TEST(VolumeManager, BringIup_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    VolumeService* volumeService = VolumeServiceSingleton::Instance();

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);


    volumeManager->Shutdown();

    delete volumeManager;
}

TEST(VolumeManager, CreateVolume_testFailedToCreateVolIfArrayIsNotMounted)
{
    // Given
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name = "volumetest";
    std::string newName = "volumetest";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;
    std::string uuid = "";

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    int expected = EID(VOL_MGR_NOT_INITIALIZED);
    int actual = volumeManager->Create(name, size, maxIops, maxBw, false, uuid);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

TEST(VolumeManager, Delete_)
{
    // Given
    std::string owner = "OWNER";
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name = "volumetest";
    std::string newName = "volumetest";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    // Then
    StateContext nextState(owner, SituationEnum::FAULT);
    volumeManager->StateChanged(nullptr, &nextState);

    int expected = EID(VOL_MGR_NOT_INITIALIZED);
    int actual = volumeManager->Delete(name);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

TEST(VolumeManager, Mount_)
{
    // Given
    std::string owner = "OWNER";
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name = "volumetest";
    std::string subnqn = "subnqn";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    // Then
    StateContext nextState(owner, SituationEnum::FAULT);
    volumeManager->StateChanged(nullptr, &nextState);

    int expected = EID(VOL_MGR_NOT_INITIALIZED);
    int actual = volumeManager->Mount(name, subnqn);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

TEST(VolumeManager, Unmount_)
{
    // Given
    std::string owner = "OWNER";
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name = "volumetest";
    std::string subnqn = "subnqn";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    // Then
    StateContext nextState(owner, SituationEnum::FAULT);
    volumeManager->StateChanged(nullptr, &nextState);

    int expected = EID(VOL_MGR_NOT_INITIALIZED);
    int actual = volumeManager->Unmount(name);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

TEST(VolumeManager, UpdateQoS_)
{
    // Given
    std::string owner = "OWNER";
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name = "volumetest";
    std::string subnqn = "subnqn";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;
    uint64_t minIops = 0;
    uint64_t minBw = 0;

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    // Then
    StateContext nextState(owner, SituationEnum::FAULT);
    volumeManager->StateChanged(nullptr, &nextState);

    int expected = EID(VOL_MGR_NOT_INITIALIZED);
    int actual = volumeManager->UpdateQoSProperty(name, maxIops, maxBw, minIops, minBw);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

TEST(VolumeManager, Rename_)
{
    // Given
    std::string owner = "OWNER";
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name = "volumetest";
    std::string newName = "newName";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    // Then
    StateContext nextState(owner, SituationEnum::FAULT);
    volumeManager->StateChanged(nullptr, &nextState);

    int expected = EID(VOL_MGR_NOT_INITIALIZED);
    int actual = volumeManager->Rename(name, newName);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

TEST(VolumeManager, GetVolumeStatus_)
{
    // Given
    std::string owner = "OWNER";
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name = "volumetest";
    std::string newName = "newName";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    // Then
    StateContext nextState(owner, SituationEnum::FAULT);
    volumeManager->StateChanged(nullptr, &nextState);

    int expected = EID(VOL_NOT_FOUND);
    int actual = volumeManager->GetVolumeStatus(0);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

TEST(VolumeManager, VolumeName_)
{
    // Given
    std::string owner = "OWNER";
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name;
    std::string newName = "newName";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    // Then
    StateContext nextState(owner, SituationEnum::FAULT);
    volumeManager->StateChanged(nullptr, &nextState);

    int expected = EID(VOL_NOT_FOUND);
    int actual = volumeManager->GetVolumeName(0, name);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

TEST(VolumeManager, VolumeID_)
{
    // Given
    std::string owner = "OWNER";
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name = "volumetest";
    std::string newName = "newName";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    // Then
    int expected = -1;
    int actual = volumeManager->GetVolumeID(name);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

TEST(VolumeManager, GetVolume_)
{
    // Given
    std::string owner = "OWNER";
    std::string arrayName = "";
    int arrayID = 0;
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    std::string name = "volumetest";
    std::string newName = "newName";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    // When
    VolumeManager* volumeManager = new VolumeManager(iArrayInfo, iState);

    // Then
    VolumeBase* expected = nullptr;
    VolumeBase* actual = volumeManager->GetVolume(0);

    ASSERT_EQ(actual, expected);

    delete volumeManager;
}

} // namespace pos
