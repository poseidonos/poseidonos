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

#include "src/metafs/include/metafs_service.h"

#include <string>

#include "test/integration-tests/metafs/metafs_test_fixture.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;

namespace pos
{
MetaFsTestFixture::MetaFsTestFixture(void)
{
    arrayId = 0;
    arrayName = "TestArray";

    tpForMetaIo = new NiceMock<MockTelemetryPublisher>;
    tpForMetafs = new NiceMock<MockTelemetryPublisher>;

    _SetArrayInfo();
    _SetThreadModel();

    storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);

    mgmt = new MetaFsManagementApi(arrayId, storage);
    ctrl = new MetaFsFileControlApi(arrayId, storage);
    io = new MetaFsIoApi(arrayId, ctrl, storage);
    wbt = new MetaFsWBTApi(arrayId, ctrl);

    metaFs = new MetaFs(arrayInfo, isLoaded, mgmt, ctrl, io, wbt, storage, tpForMetafs);
}

MetaFsTestFixture::~MetaFsTestFixture(void)
{
    delete arrayInfo;
    delete metaFs;
    MetaFsServiceSingleton::ResetInstance();
}

void
MetaFsTestFixture::_SetArrayInfo(void)
{
    memset(ptnSize, 0x0, sizeof(PartitionLogicalSize) * PartitionType::TYPE_COUNT);
    ptnSize[PartitionType::META_NVM].totalStripes = 2048;
    ptnSize[PartitionType::META_NVM].blksPerStripe = 64;
    ptnSize[PartitionType::META_SSD].totalStripes = 293888;
    ptnSize[PartitionType::META_SSD].blksPerStripe = 64;

    arrayInfo = new NiceMock<MockIArrayInfo>();

    EXPECT_CALL(*arrayInfo, GetSizeInfo(PartitionType::META_NVM)).WillRepeatedly(Return(&ptnSize[PartitionType::META_NVM]));
    EXPECT_CALL(*arrayInfo, GetSizeInfo(PartitionType::META_SSD)).WillRepeatedly(Return(&ptnSize[PartitionType::META_SSD]));
    EXPECT_CALL(*arrayInfo, GetIndex).WillRepeatedly(Return(arrayId));
    EXPECT_CALL(*arrayInfo, GetName).WillRepeatedly(Return(arrayName));
}

void
MetaFsTestFixture::_SetThreadModel(void)
{
    uint32_t coreCount = 4;
    cpu_set_t schedulerCPUSet = _GetCpuSet(0, 0);
    cpu_set_t workerCPUSet = _GetCpuSet(1, 2);

    MetaFsServiceSingleton::Instance()->Initialize(coreCount, schedulerCPUSet, workerCPUSet, tpForMetaIo);
}

cpu_set_t
MetaFsTestFixture::_GetCpuSet(int from, int to)
{
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    for (uint32_t cpu = from; cpu <= to; cpu++)
    {
        CPU_SET(cpu, &cpuSet);
    }

    return cpuSet;
}
} // namespace pos
