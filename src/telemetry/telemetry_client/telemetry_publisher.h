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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include <list>
#include "src/telemetry/telemetry_client/i_global_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_data_pool.h"
#include "src/telemetry/telemetry_id.h"
#include <string>

namespace pos
{
class TelemetryPublisher
{
public:
    TelemetryPublisher(void);
    virtual ~TelemetryPublisher(void);
    virtual void StartPublishing(void);
    virtual void StopPublishing(void);
    virtual bool IsRunning(void);
    virtual void SetMaxEntryLimit(int limit);
    virtual int GetNumEntries(void);

    virtual int PublishData(std::string id, uint32_t value);
    virtual int PublishData(std::string id, std::string value);

    virtual int CollectData(std::string id, MetricUint32& outLog);
    virtual list<MetricUint32> CollectAll(void);
    virtual void SetGlobalPublisher(IGlobalPublisher* gp);

private:
    std::string _GetTimeString(time_t time);

    IGlobalPublisher* globalPublisher;
    TelemetryDataPool dataPool;
    bool turnOn;
};

} // namespace pos
