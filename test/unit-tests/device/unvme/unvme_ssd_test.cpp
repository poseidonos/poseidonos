#include "src/device/unvme/unvme_ssd.h"

#include <gtest/gtest.h>

#include "test/unit-tests/spdk_wrapper/caller/spdk_nvme_caller_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_env_caller_mock.h"
#include "lib/spdk/lib/nvme/nvme_internal.h"

using testing::_;
using testing::NiceMock;
using testing::Return;
using namespace pos;

TEST(UnvmeSsd, testIfGetNsProperly)
{
    // Given
    NiceMock<MockSpdkNvmeCaller>* mockSpdkNvmeCaller =
        new NiceMock<MockSpdkNvmeCaller>();
    NiceMock<MockSpdkEnvCaller>* mockSpdkEnvCaller =
        new NiceMock<MockSpdkEnvCaller>();
    struct spdk_nvme_ns* ns = nullptr;
    struct spdk_nvme_ctrlr_data data;
    data.mn[0] = 0;
    data.sn[0] = 0;
    EXPECT_CALL(*mockSpdkNvmeCaller, SpdkNvmeCtrlrGetData)
        .Times(2).WillRepeatedly(Return(&data));
    UnvmeSsd unvmeSsd(
        "", 0, nullptr, ns, "", mockSpdkNvmeCaller, mockSpdkEnvCaller);

    // When
    struct spdk_nvme_ns* expected = unvmeSsd.GetNs();

    // Then
    EXPECT_EQ(expected, ns);
} // namespace pos
