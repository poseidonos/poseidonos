/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/network/nvmf_volume_pos.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "lib/spdk/lib/nvmf/nvmf_internal.h"
#include "test/unit-tests/network/nvmf_target_mock.h"
#include "test/unit-tests/network/nvmf_volume_pos_spy.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_caller_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Matcher;

namespace pos
{
class NvmfVolumePosFixture : public ::testing::Test
{
public:
    NvmfVolumePosFixture(void)
    {
    }

    virtual ~NvmfVolumePosFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        mockNvmfTarget = new NiceMock<MockNvmfTarget>(&mockSpdkCaller, false, &mockEventFrameworkApi);
    }
    virtual void
    TearDown(void)
    {
        if (nullptr != mockNvmfTarget)
        {
            delete mockNvmfTarget;
        }
    }
    void
    InitVolumeInfo(void)
    {
        vInfo = new pos_volume_info;
        vInfo->id = 0;
        snprintf(vInfo->array_name, sizeof(vInfo->array_name), "array");
    }
    void
    ResetVolumeInfo(void)
    {
        if (nullptr != vInfo)
        {
            delete vInfo;
        }
    }

protected:
    unvmf_io_handler ioHandler;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockSpdkCaller> mockSpdkCaller;
    struct pos_volume_info* vInfo;
    NiceMock<MockNvmfTarget>* mockNvmfTarget;
};

TEST_F(NvmfVolumePosFixture, NvmfVolumePos_OneArgument_Stack)
{
    NvmfVolumePos nvmfVolumePos(ioHandler);
}

TEST_F(NvmfVolumePosFixture, NvmfvolumePos_OneArgument_Heap)
{
    NvmfVolumePos* nvmfVolumePos = new NvmfVolumePos(ioHandler);

    delete nvmfVolumePos;
}

TEST_F(NvmfVolumePosFixture, NvmfVolumePos_FourArgument_Stack)
{
    NvmfVolumePos nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
}

TEST_F(NvmfVolumePosFixture, NvmfvolumePos_FourArgument_Heap)
{
    NvmfVolumePos* nvmfVolumePos = new NvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);

    delete nvmfVolumePos;
}

TEST_F(NvmfVolumePosFixture, VolumeCreated_Success)
{
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    struct spdk_bdev bdev;
    memset(&bdev, 0, sizeof(bdev));
    bool expected = true;
    InitVolumeInfo();

    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);
    ON_CALL(mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(&bdev));
    bool actual = nvmfVolumePos.VolumeCreated(vInfo);

    EXPECT_EQ(actual, expected);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeCreated_Timeout)
{
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    uint64_t time = 1000000000ULL;
    bool expected = false;
    InitVolumeInfo();

    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);
    ON_CALL(mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));
    bool actual = nvmfVolumePos.VolumeCreated(vInfo, time);

    EXPECT_EQ(actual, expected);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeDeleted_Success)
{
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    bool expected = true;
    InitVolumeInfo();

    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);
    ON_CALL(mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(nullptr));

    bool actual = nvmfVolumePos.VolumeDeleted(vInfo);

    EXPECT_EQ(actual, expected);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeDeleted_Timeout)
{
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    bool expected = false;
    uint64_t time = 1000000000ULL;
    struct spdk_bdev bdev;
    memset(&bdev, 0, sizeof(bdev));
    InitVolumeInfo();

    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);
    ON_CALL(mockSpdkCaller, SpdkBdevGetByName(_)).WillByDefault(Return(&bdev));

    bool actual = nvmfVolumePos.VolumeDeleted(vInfo, time);

    EXPECT_EQ(actual, expected);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeMounted_Success)
{
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);

    nvmfVolumePos.VolumeMounted(vInfo);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeUnmounted_Success)
{
    const char* nqn = "subnqn";
    bool expected = true;
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, CheckVolumeAttached(_, _)).WillByDefault(Return(true));
    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);
    nvmfVolumePos.SetVolumeDetachedCount(1);
    bool actual = nvmfVolumePos.VolumeUnmounted(vInfo);

    EXPECT_EQ(actual, expected);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeUnmounted_Timeout)
{
    const char* nqn = "subnqn";
    bool expected = false;
    uint64_t time = 1000000000ULL;
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, CheckVolumeAttached(_, _)).WillByDefault(Return(true));
    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);
    nvmfVolumePos.SetVolumeDetachedCount(0);
    bool actual = nvmfVolumePos.VolumeUnmounted(vInfo, time);

    EXPECT_EQ(actual, expected);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeUnmounted_DetachCountOverflow)
{
    const char* nqn = "subnqn";
    bool expected = false;
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, CheckVolumeAttached(_, _)).WillByDefault(Return(true));
    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);
    nvmfVolumePos.SetVolumeDetachedCount(2);
    bool actual = nvmfVolumePos.VolumeUnmounted(vInfo);

    EXPECT_EQ(actual, expected);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeUnmounted_VolumeAlreadyDetached)
{
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    bool expected = true;
    EXPECT_CALL(*mockNvmfTarget, CheckVolumeAttached(_, _)).WillOnce(Return(false));

    bool actual = nvmfVolumePos.VolumeUnmounted(vInfo);

    EXPECT_EQ(actual, expected);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeUpdated_Success)
{
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);

    nvmfVolumePos.VolumeUpdated(vInfo);
    ResetVolumeInfo();
}

TEST_F(NvmfVolumePosFixture, VolumeDetached_Success)
{
    vector<int> volList{0, 1, 2};
    const char* nqn = "subnqn";
    bool expected = true;
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);

    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);
    ON_CALL(mockSpdkCaller, SpdkGetAttachedSubsystemNqn(_)).WillByDefault(Return(nqn));
    ON_CALL(*mockNvmfTarget, CheckVolumeAttached(_, _)).WillByDefault(Return(true));
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));

    nvmfVolumePos.SetVolumeDetachedCount(3);

    bool actual = nvmfVolumePos.VolumeDetached(volList, "array");

    EXPECT_EQ(expected, actual);
}

TEST_F(NvmfVolumePosFixture, VolumeDetached_Timeout)
{
    vector<int> volList{0, 1, 2};
    const char* nqn = "subnqn";
    bool expected = false;
    uint64_t time = 1000000000ULL;
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);

    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, Matcher<EventFuncTwoParams>(_), _, _)).Times(1);
    ON_CALL(mockSpdkCaller, SpdkGetAttachedSubsystemNqn(_)).WillByDefault(Return(nqn));
    ON_CALL(*mockNvmfTarget, CheckVolumeAttached(_, _)).WillByDefault(Return(true));
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));

    nvmfVolumePos.SetVolumeDetachedCount(2);

    bool actual = nvmfVolumePos.VolumeDetached(volList, "array", time);

    EXPECT_EQ(expected, actual);
}

TEST_F(NvmfVolumePosFixture, VolumeDetached_NoVolumesToDetach)
{
    vector<int> volList{};
    bool expected = true;
    NvmfVolumePosSpy nvmfVolumePos(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    bool actual = nvmfVolumePos.VolumeDetached(volList, "array");

    EXPECT_EQ(expected, actual);
}

TEST_F(NvmfVolumePosFixture, _VolumeCreateHandler_Success)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return("bdev"));
    ON_CALL(*mockNvmfTarget, CreatePosBdev(_, _, _, _, _, _, _, _)).WillByDefault(Return(true));

    nvmfVolume.VolumeCreateHandler(vInfo, nullptr);
}

TEST_F(NvmfVolumePosFixture, _VolumeCreateHandler_Fail)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return("bdev"));
    ON_CALL(*mockNvmfTarget, CreatePosBdev(_, _, _, _, _, _, _, _)).WillByDefault(Return(false));

    nvmfVolume.VolumeCreateHandler(vInfo, nullptr);
}

TEST_F(NvmfVolumePosFixture, _VolumeMountHandler_GetNqnIdFail)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return(""));
    ON_CALL(*mockNvmfTarget, GetVolumeNqnId(_)).WillByDefault(Return(-1));

    nvmfVolume.VolumeMountHandler(vInfo, nullptr);
}

TEST_F(NvmfVolumePosFixture, _VolumeMountHandler_Success)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return(""));
    ON_CALL(*mockNvmfTarget, GetVolumeNqnId(_)).WillByDefault(Return(0));
    ON_CALL(*mockNvmfTarget, SetVolumeQos(_, _, _)).WillByDefault(Return());

    nvmfVolume.VolumeMountHandler(vInfo, nullptr);
}

TEST_F(NvmfVolumePosFixture, _VolumeUnmountHandler_Fail)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return(""));

    nvmfVolume.VolumeUnmountHandler(vInfo, nullptr);
}

TEST_F(NvmfVolumePosFixture, _VolumeDeleteHandler_Success)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return("bdev"));
    ON_CALL(*mockNvmfTarget, DeletePosBdev(_)).WillByDefault(Return(true));

    nvmfVolume.VolumeDeleteHandler(vInfo, nullptr);
}

TEST_F(NvmfVolumePosFixture, _VolumeDeleteHandler_Fail)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return("bdev"));
    ON_CALL(*mockNvmfTarget, DeletePosBdev(_)).WillByDefault(Return(false));

    nvmfVolume.VolumeDeleteHandler(vInfo, nullptr);
}

TEST_F(NvmfVolumePosFixture, _VolumeUpdateHandler_Success)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return("bdev"));
    ON_CALL(*mockNvmfTarget, SetVolumeQos(_, _, _)).WillByDefault(Return());

    nvmfVolume.VolumeUpdateHandler(vInfo, nullptr);
}

TEST_F(NvmfVolumePosFixture, _NamespaceDetachedHandler_Success)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return(""));

    nvmfVolume.NamespaceDetachedHandler(vInfo, NvmfCallbackStatus::SUCCESS);
}

TEST_F(NvmfVolumePosFixture, _NamespaceDetachedHandler_Fail)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    InitVolumeInfo();

    nvmfVolume.NamespaceDetachedHandler(vInfo, NvmfCallbackStatus::FAILED);
}

TEST_F(NvmfVolumePosFixture, _NamespaceDetachedAllHandler_Success)
{
    volumeListInfo* volsInfo = new volumeListInfo;
    vector<int> vols{0};
    volsInfo->subnqn = "subnqn";
    volsInfo->vols = vols;
    struct spdk_nvmf_ns ns;
    memset(&ns, 0, sizeof(ns));
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return(""));
    ON_CALL(*mockNvmfTarget, FindSubsystem(_)).WillByDefault(Return(nullptr));
    ON_CALL(*mockNvmfTarget, GetNamespace(_, _)).WillByDefault(Return(&ns));

    nvmfVolume.NamespaceDetachedAllHandler(volsInfo, NvmfCallbackStatus::SUCCESS);
}

TEST_F(NvmfVolumePosFixture, _NamespaceDetachedAllHandler_PartialFail)
{
    volumeListInfo* volsInfo = new volumeListInfo;
    vector<int> vols{0};
    volsInfo->subnqn = "subnqn";
    volsInfo->vols = vols;
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);

    ON_CALL(*mockNvmfTarget, GetBdevName(_, _)).WillByDefault(Return(""));
    ON_CALL(*mockNvmfTarget, FindSubsystem(_)).WillByDefault(Return(nullptr));
    ON_CALL(*mockNvmfTarget, GetNamespace(_, _)).WillByDefault(Return(nullptr));

    nvmfVolume.NamespaceDetachedAllHandler(volsInfo, NvmfCallbackStatus::PARTIAL_FAILED);
}

TEST_F(NvmfVolumePosFixture, _NamespaceDetachedAllHandler_Fail)
{
    volumeListInfo* volsInfo = new volumeListInfo;
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);

    nvmfVolume.NamespaceDetachedAllHandler(volsInfo, NvmfCallbackStatus::FAILED);
}

TEST_F(NvmfVolumePosFixture, _VolumeDetachHandler_Failed)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    volumeListInfo* volsInfo = new volumeListInfo;
    volsInfo->subnqn = "subnqn";

    ON_CALL(*mockNvmfTarget, DetachNamespaceAll(_, _, _)).WillByDefault(Return(false));

    nvmfVolume.VolumeDetachHandler(volsInfo, nullptr);
}

TEST_F(NvmfVolumePosFixture, _VolumeDetachedAllHandler_Success)
{
    NvmfVolumePosSpy nvmfVolume(ioHandler, &mockEventFrameworkApi, &mockSpdkCaller, mockNvmfTarget);
    volumeListInfo* volsInfo = new volumeListInfo;
    volsInfo->subnqn = "subnqn";

    ON_CALL(*mockNvmfTarget, DetachNamespaceAll(_, _, _)).WillByDefault(Return(true));

    nvmfVolume.VolumeDetachHandler(volsInfo, nullptr);
}
} // namespace pos
