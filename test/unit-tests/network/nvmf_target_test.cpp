#include "src/network/nvmf_target.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "lib/spdk/include/spdk/uuid.h"
#include "lib/spdk/lib/nvmf/nvmf_internal.h"
#include "test/unit-tests/network/nvmf_target_mock.h"
#include "test/unit-tests/network/nvmf_target_spy.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_caller_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_nvmf_caller_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_bdev_caller_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Matcher;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;

extern struct spdk_nvmf_tgt* g_spdk_nvmf_tgt;
namespace pos
{
TEST(NvmfTarget, NvmfTarget_ZeroArgument_Stack)
{
    // When: Create new object in stack
    NvmfTarget nvmfTarget;
}

TEST(NvmfTarget, NvmfTarget_FourArgument_Stack)
{
    // Given: create spdkCaller mock
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;

    // When: Create new object in stack
    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, nullptr);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, NvmfTarget_ZeroArgument_Heap)
{
    // When: Create new object in heap
    NvmfTarget* nvmfTarget = new NvmfTarget();
}

TEST(NvmfTarget, NvmfTarget_FourArgument_Heap)
{
    // Given: create spdkCaller mock
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;

    // When: Create new object in heap
    NvmfTarget* nvmfTarget = new NvmfTarget(mockSpdkCaller, false, nullptr);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CreatePosBdev_CreateBdevSuccess)
{
    // Given
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    struct spdk_bdev bdev;
    memset(&bdev, 0, sizeof(bdev));
    bool actual, expected{true};

    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));
    ON_CALL(*mockSpdkCaller, SpdkBdevCreatePosDisk(_, _, _, _, _, _, _, _)).WillByDefault(Return(&bdev));
    EXPECT_CALL(*mockSpdkCaller, SpdkUuidParse).WillOnce([=](struct spdk_uuid* uuid, const char* uuid_str)
    {
        uuid = new struct spdk_uuid;
        return 0;
    });

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    actual = nvmfTarget.CreatePosBdev("bdev", "9aa0802f-761d-4330-930d-6c77a904f5e8", 0, 1024, 512, false, "array", 0);

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CreatePosBdev_DeleteAlreadyExistingBdevAndCreateNewSuccess)
{
    // Given
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    struct spdk_bdev bdev;
    memset(&bdev, 0, sizeof(bdev));
    bool actual, expected{true};

    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(&bdev));
    EXPECT_CALL(*mockSpdkCaller, SpdkBdevDeletePosDisk(_, _, _)).Times(1);
    ON_CALL(*mockSpdkCaller, SpdkBdevCreatePosDisk(_, _, _, _, _, _, _, _)).WillByDefault(Return(&bdev));
    EXPECT_CALL(*mockSpdkCaller, SpdkUuidParse).WillOnce([=](struct spdk_uuid* uuid, const char* uuid_str)
    {
        uuid = new struct spdk_uuid;
        return 0;
    });

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    actual = nvmfTarget.CreatePosBdev("bdev", "9aa0802f-761d-4330-930d-6c77a904f5e8", 0, 1024, 512, false, "array", 0);

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CreatePosBdev_CreateBdevFail)
{
    // Given
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    bool actual, expected{false};

    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));
    ON_CALL(*mockSpdkCaller, SpdkBdevCreatePosDisk(_, _, _, _, _, _, _, _)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    actual = nvmfTarget.CreatePosBdev("bdev", "", 0, 1024, 512, false, "array", 0);

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CreatePosBdev_ParseUuidFail)
{
    // Given
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    bool actual, expected{false};

    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));
    ON_CALL(*mockSpdkCaller, SpdkBdevCreatePosDisk(_, _, _, _, _, _, _, _)).WillByDefault(Return(nullptr));
    EXPECT_CALL(*mockSpdkCaller, SpdkUuidParse).WillOnce([=](struct spdk_uuid* uuid, const char* uuid_str)
    {
        uuid = new struct spdk_uuid;
        return 1;
    });

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    actual = nvmfTarget.CreatePosBdev("bdev", "9aa0802f-761d-4330-930d-6c77a904f5e8", 0, 1024, 512, false, "array", 0);

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DeletePosBdev_DeleteBdevSuccess)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    struct spdk_bdev bdev;
    memset(&bdev, 0, sizeof(bdev));
    bool actual, expected{true};
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(&bdev));
    ON_CALL(*mockSpdkCaller, SpdkBdevDeletePosDisk(_, _, _)).WillByDefault(Return());

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    actual = nvmfTarget.DeletePosBdev("bdev");

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DeletePosBdev_DeleteBdevFail)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    bool actual, expected{false};
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    actual = nvmfTarget.DeletePosBdev("bdev");


    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DeletePosBdevAll_Timeout)
{
    NiceMock<MockEventFrameworkApi>* mockEventFrameworkApi = new NiceMock<MockEventFrameworkApi>;
    string arrayName = "POSArray";
    bool expected = false;
    EXPECT_CALL(*mockEventFrameworkApi, SendSpdkEvent(_, _, _)).Times(1);
    NvmfTarget nvmfTarget(nullptr, false, mockEventFrameworkApi);

    bool actual = nvmfTarget.DeletePosBdevAll(arrayName, 500000000ULL);
    ASSERT_EQ(actual, expected);
    delete mockEventFrameworkApi;
}

TEST(NvmfTarget, _DeletePosBdevHandler_Success)
{
    NiceMock<MockSpdkCaller> mockSpdkCaller;
    NiceMock<MockSpdkBdevCaller> mockSpdkBdevCaller;
    string arrayName = "POSArray";
    char bdevName[16] = "bdev_0_POSArray";
    struct spdk_bdev bdev;
    memset(&bdev, 0, sizeof(bdev));
    bdev.name = "bdev_0_POSArray";

    EXPECT_CALL(mockSpdkBdevCaller, SpdkBdevFirst()).WillOnce(Return(&bdev));
    EXPECT_CALL(mockSpdkBdevCaller, SpdkBdevNext)
        .WillOnce(Return(nullptr));
    EXPECT_CALL(mockSpdkCaller, SpdkBdevDeletePosDisk).Times(1);
    NvmfTargetSpy nvmfTarget(nullptr, false, nullptr);
    nvmfTarget.DeletePosBdevAllHandler(&arrayName, &mockSpdkCaller, &mockSpdkBdevCaller);
}

TEST(NvmfTarget, TryToAttachNamespace_Fail)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockEventFrameworkApi>* mockEventFrameworkApi = new NiceMock<MockEventFrameworkApi>;
    string nqn{"test_nqn"};
    string arrayName{"array"};
    bool actual, expected{false};

    ON_CALL(*mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).WillByDefault(Return(true));
    NvmfTarget nvmfTarget(mockSpdkCaller, false, mockEventFrameworkApi);
    actual = nvmfTarget.TryToAttachNamespace(nqn, 0, arrayName, 500000000ULL);
    ASSERT_EQ(actual, expected);
    delete mockEventFrameworkApi;
    delete mockSpdkCaller;
}

TEST(NvmfTarget, AttachNamespace_TargetDoesNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    string nqn{"test_nqn"};
    string bdevName{"bdev"};
    bool expected{false};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    bool actual = nvmfTarget.AttachNamespace(nqn, bdevName, nullptr, nullptr);

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetNamespace_BdevDoesNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    string bdevName{"bdev"};
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    struct spdk_nvmf_ns* actual = nvmfTarget.GetNamespace(nullptr, bdevName);
    ASSERT_EQ(actual, nullptr);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetNamespace_NamespaceDoesNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    string bdevName{"bdev"};
    struct spdk_bdev* targetBdev;
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(targetBdev));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    struct spdk_nvmf_ns* actual = nvmfTarget.GetNamespace(nullptr, bdevName);
    ASSERT_EQ(actual, nullptr);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetNamespace_Success_Iter)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    string bdevName{"bdev"};
    struct spdk_nvmf_ns* ns[1];
    ns[0] = reinterpret_cast<struct spdk_nvmf_ns*>(0x1); // to make it non-null
    struct spdk_bdev* targetBdev[1];
    targetBdev[0] = reinterpret_cast<struct spdk_bdev*>(0x1); // to make it non-null
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(targetBdev[0]));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(ns[0]));
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfNsGetBdev(_))
        .WillOnce(Return(nullptr))
        .WillOnce(Return(targetBdev[0]));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNextNs(_, _)).WillByDefault(Return(ns[0]));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    struct spdk_nvmf_ns* actual = nvmfTarget.GetNamespace(nullptr, bdevName);
    ASSERT_EQ(actual, ns[0]);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DetachNamespace_TargetNotExist)
{
    string nqn{"subnqn"};
    uint32_t nsid = 0;
    bool expected{false};

    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    bool actual = nvmfTarget.DetachNamespace(nqn, nsid, nullptr, nullptr);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DetachNamespace_NqnEmpty)
{
    struct spdk_nvmf_tgt* target;
    g_spdk_nvmf_tgt = target;
    string nqn{""};
    uint32_t nsid = 0;
    bool expected{false};

    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    bool actual = nvmfTarget.DetachNamespace(nqn, nsid, nullptr, nullptr);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DetachNamespace_FailedToFindSubsystem)
{
    struct spdk_nvmf_tgt* target;
    g_spdk_nvmf_tgt = target;
    string nqn{"subnqn"};
    uint32_t nsid = 0;
    bool expected{false};
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    bool actual = nvmfTarget.DetachNamespace(nqn, nsid, nullptr, nullptr);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DetachNamespace_FailedToGetNs)
{
    struct spdk_nvmf_tgt* target[1];
    g_spdk_nvmf_tgt = target[0];
    string nqn{"subnqn"};
    uint32_t nsid = 0;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    bool expected{false};
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));
    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    struct pos_volume_info* vInfo = new pos_volume_info;
    vInfo->id = 0;
    snprintf(vInfo->array_name, sizeof(vInfo->array_name), "array");
    bool actual = nvmfTarget.DetachNamespace(nqn, nsid, nullptr, vInfo);
    ASSERT_EQ(actual, expected);
    delete vInfo;
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DetachNamespace_FailedToGetNs2)
{
    struct spdk_nvmf_tgt* target[1];
    g_spdk_nvmf_tgt = target[0];
    string nqn{"subnqn"};
    uint32_t nsid = 0;
    bool expected{false};
    struct spdk_bdev* bdev;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(bdev));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(nullptr));
    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    struct pos_volume_info* vInfo = new pos_volume_info;
    vInfo->id = 0;
    snprintf(vInfo->array_name, sizeof(vInfo->array_name), "array");
    bool actual = nvmfTarget.DetachNamespace(nqn, nsid, nullptr, vInfo);
    ASSERT_EQ(actual, expected);
    delete vInfo;
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DetachNamespace_GetNsSuccess)
{
    struct spdk_nvmf_tgt target;
    memset(&target, 0, sizeof(target));
    g_spdk_nvmf_tgt = &target;

    string nqn{"subnqn"};
    uint32_t nsid = 0;
    bool expected{true};

    struct spdk_bdev bdev;
    memset(&bdev, 0, sizeof(bdev));

    struct spdk_nvmf_ns ns;
    memset(&ns, 0, sizeof(ns));

    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));

    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(&bdev));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(&ns));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfNsGetBdev(_)).WillByDefault(Return(nullptr));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNextNs(_, _)).WillByDefault(Return(&ns));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfNsGetBdev(_)).WillByDefault(Return(&bdev));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfNsGetId(_)).WillByDefault(Return(1));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemPause(_, _, _, _)).WillByDefault(Return(0));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    struct pos_volume_info* vInfo = new pos_volume_info;
    vInfo->id = 0;
    snprintf(vInfo->array_name, sizeof(vInfo->array_name), "array");
    bool actual = nvmfTarget.DetachNamespace(nqn, nsid, nullptr, vInfo);
    ASSERT_EQ(actual, expected);
    delete vInfo;
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CheckSubsystemExistance_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_tgt* nvmf_tgt;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfGetTgt(_)).WillByDefault(Return(nvmf_tgt));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirst(_)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetType(_)).WillByDefault(Return(SPDK_NVMF_SUBTYPE_NVME));
    bool expected{true};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    bool actual = nvmfTarget.CheckSubsystemExistance();
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CheckSubsystemExistance_FailedToGetSubsystem)
{
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    struct spdk_nvmf_tgt* nvmf_tgt;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfGetTgt(_)).WillByDefault(Return(nvmf_tgt));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirst(_)).WillByDefault(Return(nullptr));
    bool expected{false};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    bool actual = nvmfTarget.CheckSubsystemExistance();
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CheckSubsystemExistance_IterSubsystem)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_tgt* nvmf_tgt;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfGetTgt(_)).WillByDefault(Return(nvmf_tgt));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirst(_)).WillByDefault(Return(&subsystem));
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetType(_))
        .WillOnce(Return(SPDK_NVMF_SUBTYPE_DISCOVERY))
        .WillOnce(Return(SPDK_NVMF_SUBTYPE_NVME));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNext(_)).WillByDefault(Return(&subsystem));
    bool expected{true};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    bool actual = nvmfTarget.CheckSubsystemExistance();
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DetachNamespaceAll_Sucess)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_tgt* nvmf_tgt[1];
    g_spdk_nvmf_tgt = nvmf_tgt[0];
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemPause(_, _, _, _)).WillByDefault(Return(0));
    bool expected{true};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    bool actual = nvmfTarget.DetachNamespaceAll("subnqn", nullptr, nullptr);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DetachNamespaceAll_TargetNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    g_spdk_nvmf_tgt = nullptr;
    bool expected{false};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    bool actual = nvmfTarget.DetachNamespaceAll("subnqn", nullptr, nullptr);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, DetachNamespaceAll_SubsystemNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_tgt* nvmf_tgt[1];
    g_spdk_nvmf_tgt = nvmf_tgt[0];
    bool expected{false};

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    bool actual = nvmfTarget.DetachNamespaceAll("subnqn", nullptr, nullptr);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetSubsystemNsCnt_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_ns ns;
    memset(&ns, 0, sizeof(ns));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(&ns));
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNextNs(_, _))
        .WillOnce(Return(&ns))
        .WillOnce(Return(nullptr));
    uint32_t expected{2};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    uint32_t actual = nvmfTarget.GetSubsystemNsCnt(nullptr);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, AllocateSubsystem_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirst(_)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNqn(_)).WillByDefault(Return("subnqn"));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetType(_)).WillByDefault(Return(SPDK_NVMF_SUBTYPE_NVME));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string arrayName = "POSArray";
    struct spdk_nvmf_subsystem* actual = nvmfTarget.AllocateSubsystem(arrayName, 0);
    ASSERT_EQ(actual, &subsystem);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, AllocateSubsystem_AllocateNextSubsystem)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    struct spdk_nvmf_ns ns;
    memset(&ns, 0, sizeof(ns));
    string subnqn = "subnqn";

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirst(_)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNqn(_)).WillByDefault(Return(subnqn.c_str()));

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetType(_)).WillByDefault(Return(SPDK_NVMF_SUBTYPE_NVME));
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_))
        .WillOnce(Return(&ns))
        .WillOnce(Return(nullptr));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNextNs(_, _)).WillByDefault(Return(nullptr));
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNext(_))
        .WillOnce(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string arrayName = "POSArray";
    nvmfTarget.SetSubsystemArrayName(subnqn, arrayName);
    struct spdk_nvmf_subsystem* actual = nvmfTarget.AllocateSubsystem(arrayName, 0);
    ASSERT_EQ(actual, &subsystem);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, AllocateSubsystem_FailToAllocateSubsystem)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirst(_)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNqn(_)).WillByDefault(Return("subnqn"));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetType(_)).WillByDefault(Return(SPDK_NVMF_SUBTYPE_DISCOVERY));
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNext(_))
        .WillOnce(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string arrayName = "POSArray";
    struct spdk_nvmf_subsystem* actual = nvmfTarget.AllocateSubsystem(arrayName, 0);
    ASSERT_EQ(actual, nullptr);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, AllocateSubsystem_FailToGetFirstSubsystem)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_subsystem* expected = nullptr;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirst(_)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string arrayName = "POSArray";
    struct spdk_nvmf_subsystem* actual = nvmfTarget.AllocateSubsystem(arrayName, 0);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetBdevName_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    string expected = "bdev_0_array";

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    string actual = nvmfTarget.GetBdevName(0, "array");
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetVolumeNqn_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNqn(_)).WillByDefault(Return("subnqn"));
    string expected = "subnqn";

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string actual = nvmfTarget.GetVolumeNqn(nullptr);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetVolumeNqnId_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    int32_t id = 1;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkCaller, SpdkNvmfSubsystemGetId(_)).WillByDefault(Return(id));
    uint32_t expected = id;

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string subnqn = "subnqn";
    int32_t actual = nvmfTarget.GetVolumeNqnId(subnqn);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetVolumeNqnId_Fail)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(nullptr));
    int32_t expected = -1;

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    int32_t actual = nvmfTarget.GetVolumeNqnId("subnqn");

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, FindSubsystem_success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    struct spdk_nvmf_subsystem* actual = nvmfTarget.FindSubsystem("");

    ASSERT_EQ(actual, &subsystem);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, FindSubsystem_Fail)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(nullptr));
    struct spdk_nvmf_subsystem* expected = nullptr;

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    struct spdk_nvmf_subsystem* actual = nvmfTarget.FindSubsystem("");

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, SetVolumeQos_success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    struct spdk_bdev* bdev[1];
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(bdev[0]));
    ON_CALL(*mockSpdkCaller, SpdkBdevSetQosRateLimits(_, _, _, _)).WillByDefault(Return());

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    nvmfTarget.SetVolumeQos("bdev", 0, 0);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, SetVolumeQos_Fail)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    nvmfTarget.SetVolumeQos("bdev", 0, 0);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, SetVolumeQos_feQosEnableTrue)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;

    NvmfTarget nvmfTarget(mockSpdkCaller, true, nullptr);
    nvmfTarget.SetVolumeQos("bdev", 0, 0);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, QosEnableDone_Empty)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    nvmfTarget.QosEnableDone(nullptr, 0);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetHostNqn_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    struct spdk_nvmf_ctrlr* ctrlr[1];
    string hostNqn = "hostnqn";
    vector<string> expected;
    expected.push_back(hostNqn);
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkCaller, SpdkNvmfSubsystemGetFirstCtrlr(_)).WillByDefault(Return(ctrlr[0]));
    ON_CALL(*mockSpdkCaller, SpdkNvmfSubsystemGetCtrlrHostnqn(_)).WillByDefault(Return(const_cast<char*>(hostNqn.c_str())));
    ON_CALL(*mockSpdkCaller, SpdkNvmfSubsystemGetNextCtrlr(_, _)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    vector<string> actual = nvmfTarget.GetHostNqn("subnqn");
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CheckVolumeAttached_success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    const char* nqn = "subnqn";
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    struct spdk_bdev* bdev[1];
    struct spdk_nvmf_ns ns;
    memset(&ns, 0, sizeof(ns));
    ON_CALL(*mockSpdkCaller, SpdkGetAttachedSubsystemNqn(_)).WillByDefault(Return(nqn));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(bdev[0]));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(&ns));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfNsGetBdev(_)).WillByDefault(Return(bdev[0]));
    bool expected{true};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    bool actual = nvmfTarget.CheckVolumeAttached(0, "array");
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CheckVolumeAttached_nqnNull)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    const char* nqn = nullptr;
    ON_CALL(*mockSpdkCaller, SpdkGetAttachedSubsystemNqn(_)).WillByDefault(Return(nqn));
    bool expected{false};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    bool actual = nvmfTarget.CheckVolumeAttached(0, "array");
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CheckVolumeAttached_nqnEmpty)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    const char* nqn = "";
    ON_CALL(*mockSpdkCaller, SpdkGetAttachedSubsystemNqn(_)).WillByDefault(Return(nqn));
    bool expected{false};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    bool actual = nvmfTarget.CheckVolumeAttached(0, "array");
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CheckVolumeAttached_SubsystemNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    const char* nqn = "subnqn";
    ON_CALL(*mockSpdkCaller, SpdkGetAttachedSubsystemNqn(_)).WillByDefault(Return(nqn));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(nullptr));
    bool expected{false};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    bool actual = nvmfTarget.CheckVolumeAttached(0, "array");
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, CheckVolumeAttached_NsNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    const char* nqn = "subnqn";
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    ON_CALL(*mockSpdkCaller, SpdkGetAttachedSubsystemNqn(_)).WillByDefault(Return(nqn));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));
    bool expected{false};

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    bool actual = nvmfTarget.CheckVolumeAttached(0, "array");
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetAttachedVolumeList_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    vector<pair<int, string>> expected;
    expected.push_back(make_pair(0, "array"));
    struct spdk_nvmf_subsystem* subsystem[0];
    subsystem[0] = reinterpret_cast<struct spdk_nvmf_subsystem*>(0x1); // to make it non-null
    struct spdk_nvmf_ns* ns[1];
    ns[0] = reinterpret_cast<struct spdk_nvmf_ns*>(0x1); // to make it non-null
    struct spdk_bdev* bdev;
    string bdevName = "bdev_0_array";
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(subsystem[0]));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(ns[0]));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfNsGetBdev(_)).WillByDefault(Return(bdev));
    ON_CALL(*mockSpdkCaller, SpdkBdevGetName(_)).WillByDefault(Return(bdevName.c_str()));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNextNs(_, _)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string subnqn = "subnqn";
    vector<pair<int, string>> actual = nvmfTarget.GetAttachedVolumeList(subnqn);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetAttachedVolumeList_SubsystemNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    vector<pair<int, string>> expected;
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string subnqn = "subnqn";
    vector<pair<int, string>> actual = nvmfTarget.GetAttachedVolumeList(subnqn);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetAttachedVolumeList_FailToFindVolumdId)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    vector<pair<int, string>> expected;
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    struct spdk_nvmf_ns ns;
    memset(&ns, 0, sizeof(ns));
    struct spdk_bdev* bdev;
    string bdevName = "bdev";
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(&ns));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfNsGetBdev(_)).WillByDefault(Return(bdev));
    ON_CALL(*mockSpdkCaller, SpdkBdevGetName(_)).WillByDefault(Return(bdevName.c_str()));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNextNs(_, _)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string subnqn = "subnqn";
    vector<pair<int, string>> actual = nvmfTarget.GetAttachedVolumeList(subnqn);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetAttachedVolumeList_FailToFindArrayName)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    vector<pair<int, string>> expected;
    expected.push_back(make_pair(2, "array2"));
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    struct spdk_nvmf_ns ns;
    memset(&ns, 0, sizeof(ns));
    struct spdk_bdev* bdev;
    string bdevName = "bdev_1";
    string bdevName2 = "bdev_2_array2";
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(&ns));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfNsGetBdev(_)).WillByDefault(Return(bdev));
    EXPECT_CALL(*mockSpdkCaller, SpdkBdevGetName(_))
        .WillOnce(Return(bdevName.c_str()))
        .WillOnce(Return(bdevName2.c_str()));
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNextNs(_, _))
        .WillOnce(Return(&ns))
        .WillOnce(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    string subnqn = "subnqn";
    vector<pair<int, string>> actual = nvmfTarget.GetAttachedVolumeList(subnqn);
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetPosBdevUuid_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    struct spdk_bdev* bdev[1];
    struct spdk_uuid* uuid[1];
    string expected = "abcd";
    char uuidString[37] = "abcd";

    EXPECT_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillOnce(Return(bdev[0]));
    EXPECT_CALL(*mockSpdkCaller, SpdkBdevGetUuid(_)).WillOnce(Return(uuid[0]));
    EXPECT_CALL(*mockSpdkCaller, SpdkUuidFmtLower(_, _, _)).WillOnce([&](char* uuidStr, size_t uuidStrSize, const spdk_uuid* uuid)
    {
        memset(uuidStr, 0, uuidStrSize);
        memcpy(uuidStr, uuidString, 5);
        return 0;
    });

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    string actual = nvmfTarget.GetPosBdevUuid(0, "array");
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetPosBdevUuid_BdevNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    string expected = "";

    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));
    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    string actual = nvmfTarget.GetPosBdevUuid(0, "array");

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetPosBdevUuid_BdevUuidNotExist)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    struct spdk_bdev* bdev[1];
    string expected = "";

    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(bdev[0]));
    ON_CALL(*mockSpdkCaller, SpdkBdevGetUuid(_)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    string actual = nvmfTarget.GetPosBdevUuid(0, "array");

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, GetPosBdevUuid_FailToConvertUuidToString)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    struct spdk_bdev* bdev[1];
    struct spdk_uuid* uuid[1];

    string expected = "";

    ON_CALL(*mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(bdev[0]));
    ON_CALL(*mockSpdkCaller, SpdkBdevGetUuid(_)).WillByDefault(Return(uuid[0]));
    ON_CALL(*mockSpdkCaller, SpdkUuidFmtLower(_, _, _)).WillByDefault(Return(-1));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr);
    string actual = nvmfTarget.GetPosBdevUuid(0, "array");

    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTarget, SetSubsystemArrayName_Success)
{
    string subnqn = "subnqn";
    string arrayName = "POSArray";
    NvmfTarget nvmfTarget(nullptr, false, nullptr);
    bool actual = nvmfTarget.SetSubsystemArrayName(subnqn, arrayName);
    bool expected = true;

    ASSERT_EQ(actual, expected);
}

TEST(NvmfTarget, SetSubsystemArrayName_Fail)
{
    string subnqn = "subnqn";
    string arrayName1 = "POSArray1";
    string arrayName2 = "POSArray2";
    NvmfTarget nvmfTarget(nullptr, false, nullptr);
    nvmfTarget.SetSubsystemArrayName(subnqn, arrayName1);
    bool actual = nvmfTarget.SetSubsystemArrayName(subnqn, arrayName2);
    bool expected = false;
    ASSERT_EQ(actual, expected);
}

TEST(NvmfTarget, GetSubsystemArrayName_Success)
{
    string subnqn = "subnqn";
    string arrayName = "POSArray";
    NvmfTarget nvmfTarget(nullptr, false, nullptr);
    nvmfTarget.SetSubsystemArrayName(subnqn, arrayName);
    string actual = nvmfTarget.GetSubsystemArrayName(subnqn);

    ASSERT_EQ(actual, arrayName);
}

TEST(NvmfTarget, GetSubsystemArrayName_Fail)
{
    string subnqn1 = "subnqn1";
    string subnqn2 = "subnqn2";
    string arrayName = "POSArray";
    NvmfTarget nvmfTarget(nullptr, false, nullptr);
    nvmfTarget.SetSubsystemArrayName(subnqn1, arrayName);
    string actual = nvmfTarget.GetSubsystemArrayName(subnqn2);
    string expected = "";
    ASSERT_EQ(actual, expected);
}

TEST(NvmfTarget, RemoveSubsystemArrayName_Success)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    string subnqn = "subnqn";
    string arrayName = "POSArray";
    struct spdk_nvmf_subsystem subsystem;
    memset(&subsystem, 0, sizeof(subsystem));

    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfTgtFindSubsystem(_, _)).WillByDefault(Return(&subsystem));
    ON_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetFirstNs(_)).WillByDefault(Return(nullptr));

    NvmfTarget nvmfTarget(mockSpdkCaller, false, nullptr, mockSpdkNvmfCaller);
    nvmfTarget.SetSubsystemArrayName(subnqn, arrayName);
    nvmfTarget.RemoveSubsystemArrayName(subnqn);

    string actual = nvmfTarget.GetSubsystemArrayName(subnqn);
    string expected = "";
    ASSERT_EQ(actual, expected);
    delete mockSpdkCaller;
}

TEST(NvmfTargetSpdk, NvmfTargetSpdk_AttachNamespacePauseDone)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NvmfTargetSpy nvmfTarget(mockSpdkCaller, false, nullptr);
    nvmfTarget.AttachNamespacePauseDone(nullptr, nullptr, NvmfCallbackStatus::FAILED);
}

TEST(NvmfTargetSpdk, NvmfTargetSpdk_DetachNamespacePauseDone)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NvmfTargetSpy nvmfTarget(mockSpdkCaller, false, nullptr);
    nvmfTarget.DetachNamespacePauseDone(nullptr, nullptr, NvmfCallbackStatus::FAILED);
}

TEST(NvmfTargetSpdk, NvmfTargetSpdk_DetachNamespaceAllPauseDone)
{
    NiceMock<MockSpdkCaller>* mockSpdkCaller = new NiceMock<MockSpdkCaller>;
    NvmfTargetSpy nvmfTarget(mockSpdkCaller, false, nullptr);
    nvmfTarget.DetachNamespaceAllPauseDone(nullptr, nullptr, NvmfCallbackStatus::FAILED);
}

TEST(NvmfTarget, AttachNamespaceWithPause_Success)
{
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    NiceMock<MockEventFrameworkApi>* mockEventFrameworkApi = new NiceMock<MockEventFrameworkApi>;
    void* arg1 = nullptr;
    void* arg2 = nullptr;
    NvmfTargetSpy nvmfTarget(nullptr, false, nullptr);
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemPause(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncFourParams>(_), _, _)).Times(1);

    nvmfTarget.AttachNamespaceWithPause(arg1, arg2, mockEventFrameworkApi, mockSpdkNvmfCaller);
    delete mockEventFrameworkApi;
}

TEST(NvmfTarget, DetachNamespaceWithPause_Success)
{
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    NiceMock<MockEventFrameworkApi>* mockEventFrameworkApi = new NiceMock<MockEventFrameworkApi>;
    void* arg1 = nullptr;
    void* arg2 = nullptr;
    NvmfTargetSpy nvmfTarget(nullptr, false, nullptr);
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemPause(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncFourParams>(_), _, _)).Times(1);

    nvmfTarget.DetachNamespaceWithPause(arg1, arg2, mockEventFrameworkApi, mockSpdkNvmfCaller);
    delete mockEventFrameworkApi;
}

TEST(NvmfTarget, DetachNamespaceAllWithPause_Success)
{
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    NiceMock<MockEventFrameworkApi>* mockEventFrameworkApi = new NiceMock<MockEventFrameworkApi>;
    void* arg1 = nullptr;
    void* arg2 = nullptr;
    NvmfTargetSpy nvmfTarget(nullptr, false, nullptr);
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemPause(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfSubsystemGetNqn(_)).Times(1);
    EXPECT_CALL(*mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncFourParams>(_), _, _)).Times(1);

    nvmfTarget.DetachNamespaceAllWithPause(arg1, arg2, mockEventFrameworkApi, mockSpdkNvmfCaller);
    delete mockEventFrameworkApi;
}
TEST(NvmfTarget, SpdkNvmfInitializeNumaAwarePollGroup)
{
    NiceMock<MockSpdkNvmfCaller>* mockSpdkNvmfCaller = new NiceMock<MockSpdkNvmfCaller>;
    EXPECT_CALL(*mockSpdkNvmfCaller, SpdkNvmfInitializeNumaAwarePollGroup()).Times(0);
    delete mockSpdkNvmfCaller;
}

} // namespace pos
