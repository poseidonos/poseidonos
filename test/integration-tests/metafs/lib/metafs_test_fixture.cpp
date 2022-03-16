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

#include "test/integration-tests/metafs/lib/metafs_test_fixture.h"

#include <string>

#include "src/metafs/include/metafs_service.h"

using ::testing::_;
using ::testing::Matcher;
using ::testing::NiceMock;
using ::testing::Return;

using namespace std;

namespace pos
{
MetaFsTestFixture::MetaFsTestFixture(void)
: tpForMetaIo(new NiceMock<MockTelemetryPublisher>)
{
    tpForMetaIo = new NiceMock<MockTelemetryPublisher>;
    config = new NiceMock<MockMetaFsConfigManager>(nullptr);

    EXPECT_CALL(*config, Init).WillRepeatedly(Return(true));
    EXPECT_CALL(*config, GetMioPoolCapacity).WillRepeatedly(Return(32 * 1024));
    EXPECT_CALL(*config, GetMpioPoolCapacity).WillRepeatedly(Return(32 * 1024));
    EXPECT_CALL(*config, IsWriteMpioCacheEnabled).WillRepeatedly(Return(true));
    EXPECT_CALL(*config, GetWriteMpioCacheCapacity).WillRepeatedly(Return(32));
    EXPECT_CALL(*config, IsDirectAccessEnabled).WillRepeatedly(Return(true));
    EXPECT_CALL(*config, GetTimeIntervalInMillisecondsForMetric).WillRepeatedly(Return(5000));

    _SetThreadModel();

    for (size_t arrayId = 0; arrayId < ARRAY_COUNT; ++arrayId)
    {
        ArrayComponents* comp = new ArrayComponents;

        _SetArrayInfo(comp, arrayId);
        comp->tpForMetafs = new NiceMock<MockTelemetryPublisher>;
        comp->storage = new TestMetaStorageSubsystem(arrayId);
        comp->mgmt = new MetaFsManagementApi(arrayId, comp->storage);
        comp->ctrl = new MetaFsFileControlApi(arrayId, comp->storage);
        comp->io = new MetaFsIoApi(arrayId, comp->ctrl, comp->storage, comp->tpForMetafs);
        comp->wbt = new MetaFsWBTApi(arrayId, comp->ctrl);
        comp->metaFs = new MetaFs(comp->arrayInfo, comp->isLoaded, comp->mgmt,
            comp->ctrl, comp->io, comp->wbt, comp->storage, comp->tpForMetafs);

        components.emplace_back(comp);
    }
}

MetaFsTestFixture::~MetaFsTestFixture(void)
{
    for (auto& comp : components)
    {
        delete comp->arrayInfo;
        delete comp->metaFs;
        delete comp;
    }
    components.clear();
    delete config;
    MetaFsServiceSingleton::ResetInstance();
}

void
MetaFsTestFixture::_SetArrayInfo(ArrayComponents* component, const int index)
{
    component->isLoaded = false;
    component->arrayId = index;
    component->arrayName = "TestArray" + std::to_string(index);

    memset(component->ptnSize, 0x0, sizeof(PartitionLogicalSize) * PartitionType::TYPE_COUNT);

    component->ptnSize[PartitionType::META_NVM].totalStripes = 1024;
    component->ptnSize[PartitionType::META_NVM].blksPerStripe = 32;
    component->ptnSize[PartitionType::META_SSD].totalStripes = 2048;
    component->ptnSize[PartitionType::META_SSD].blksPerStripe = 32;
    component->ptnSize[PartitionType::JOURNAL_SSD].totalStripes = 1024;
    component->ptnSize[PartitionType::JOURNAL_SSD].blksPerStripe = 32;

    component->arrayInfo = new NiceMock<MockIArrayInfo>();

    EXPECT_CALL(*component->arrayInfo, GetSizeInfo(PartitionType::META_NVM)).WillRepeatedly(Return(&component->ptnSize[PartitionType::META_NVM]));
    EXPECT_CALL(*component->arrayInfo, GetSizeInfo(PartitionType::META_SSD)).WillRepeatedly(Return(&component->ptnSize[PartitionType::META_SSD]));
    EXPECT_CALL(*component->arrayInfo, GetSizeInfo(PartitionType::JOURNAL_SSD)).WillRepeatedly(Return(&component->ptnSize[PartitionType::JOURNAL_SSD]));
    EXPECT_CALL(*component->arrayInfo, GetIndex).WillRepeatedly(Return(component->arrayId));
    EXPECT_CALL(*component->arrayInfo, GetName).WillRepeatedly(Return(component->arrayName));
}

void
MetaFsTestFixture::_SetThreadModel(void)
{
    uint32_t coreCount = 4;
    cpu_set_t schedulerCPUSet = _GetCpuSet(0, 0);
    cpu_set_t workerCPUSet = _GetCpuSet(1, 1);

    MetaFsServiceSingleton::Instance(nullptr, config)
        ->Initialize(coreCount, schedulerCPUSet, workerCPUSet, tpForMetaIo);
}

cpu_set_t
MetaFsTestFixture::_GetCpuSet(const int from, const int to)
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
