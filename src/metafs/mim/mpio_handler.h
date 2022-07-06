/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include <string>

#include "mpio_allocator.h"
#include "src/metafs/include/mf_property.h"
#include "src/metafs/lib/metafs_time_interval.h"
#include "src/metafs/mim/metafs_io_wrr_q.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
class MetaFsConfigManager;

class MpioHandler
{
public:
    explicit MpioHandler(const int threadId, const int coreId,
        MetaFsConfigManager* configManager, TelemetryPublisher* tp = nullptr,
        MetaFsIoWrrQ<Mpio*, MetaFileType>* doneQ = nullptr);
    virtual ~MpioHandler(void);

    virtual void EnqueuePartialMpio(Mpio* mpio);
    virtual void BindMpioAllocator(MpioAllocator* mpioAllocator);
    virtual void BottomhalfMioProcessing(void);

private:
    void _UpdateMetricsConditionally(Mpio* mpio);
    void _PublishPeriodicMetrics(void);

    MetaFsIoWrrQ<Mpio*, MetaFileType>* partialMpioDoneQ;
    MpioAllocator* mpioAllocator;
    int coreId;
    TelemetryPublisher* telemetryPublisher;
    int64_t sampledTimeSpentProcessingAllStages;
    int64_t sampledTimeSpentFromWriteToRelease;
    int64_t sampledTimeSpentFromPushToPop;
    int64_t totalProcessedMpioCount;
    int64_t sampledProcessedMpioCount;
    MetaFsTimeInterval metaFsTimeInterval;
    size_t skipCount;
    const size_t SAMPLING_SKIP_COUNT;

    static const uint32_t NUM_STORAGE = (int)MetaStorageType::Max;
    static const uint32_t NUM_FILE_TYPE = (int)MetaFileType::MAX;
    int64_t doneCountByStorage[NUM_STORAGE];
    int64_t doneCountByFileType[NUM_FILE_TYPE];
};
} // namespace pos
