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

#pragma once

#include "src/metafs/metafs.h"

#include <string>

#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"

using ::testing::NiceMock;

namespace pos
{
class MetaFsTestFixture
{
public:
    MetaFsTestFixture(void);
    virtual ~MetaFsTestFixture(void);

protected:
    NiceMock<MockIArrayInfo>* arrayInfo = nullptr;

    MetaFs* metaFs = nullptr;

    MetaFsManagementApi* mgmt = nullptr;
    MetaFsFileControlApi* ctrl = nullptr;
    MetaFsIoApi* io = nullptr;
    MetaFsWBTApi* wbt = nullptr;
    NiceMock<MockMetaStorageSubsystem>* storage = nullptr;
    NiceMock<MockTelemetryPublisher>* tp = nullptr;

    bool isLoaded = false;
    int arrayId = INT32_MAX;
    std::string arrayName = "";
    PartitionLogicalSize ptnSize[PartitionType::PARTITION_TYPE_MAX];

private:
    void _SetArrayInfo(void);
    void _SetThreadModel(void);
    cpu_set_t _GetCpuSet(int from, int to);
};

} // namespace pos
