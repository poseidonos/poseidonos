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

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "src/metafs/metafs.h"
#include "test/integration-tests/metafs/lib/test_meta_storage_subsystem.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::NiceMock;

namespace pos
{
class MetaFsTestFixture
{
public:
    MetaFsTestFixture(void);
    virtual ~MetaFsTestFixture(void);
    virtual MetaFs* GetMetaFs(const int arrayId) const
    {
        return components.at(arrayId)->metaFs;
    }

    static const size_t ARRAY_COUNT = 2;

protected:
    NiceMock<MockTelemetryPublisher>* tpForMetaIo;

    struct ArrayComponents
    {
        NiceMock<MockIArrayInfo>* arrayInfo;
        MetaFs* metaFs;
        MetaFsManagementApi* mgmt;
        MetaFsFileControlApi* ctrl;
        MetaFsIoApi* io;
        MetaFsWBTApi* wbt;
        TestMetaStorageSubsystem* storage;
        NiceMock<MockTelemetryPublisher>* tpForMetafs;

        bool isLoaded;
        int arrayId;
        std::string arrayName;
        PartitionLogicalSize ptnSize[PartitionType::TYPE_COUNT];
    };

    std::vector<ArrayComponents*> components;

private:
    void _SetArrayInfo(ArrayComponents* component, const int index);
    void _SetThreadModel(void);
    cpu_set_t _GetCpuSet(const int from, const int to);
};

} // namespace pos
