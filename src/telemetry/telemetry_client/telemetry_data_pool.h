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

#include <map>
#include <chrono>
#include <iomanip>
#include <list>
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_data_pool.h"
#include <sstream>
#include <string>

namespace pos
{
class TelemetryGeneralMetric
{
public:
    TelemetryGeneralMetric(void) {}
    ~TelemetryGeneralMetric(void) {}
    TelemetryGeneralMetric(tm t, uint32_t v)
    {
        loggedTime = t;
        std::ostringstream oss;
        oss << std::put_time(&loggedTime, "%Y-%m-%d %H:%M:%S");
        loggedStrTime = oss.str();
        value = v;
    }
    std::string GetTimeString(void) { return loggedStrTime; }
    tm GetTime(void) { return loggedTime; }
    uint32_t GetValue(void) { return value; }
    std::string GetId(void) { return id; }
    void Set(tm t, uint32_t v) { loggedTime = t; value = v; }

private:
    uint32_t value;
    tm loggedTime;
    std::string loggedStrTime;
    std::string id;
};

class TelemetryDataPool
{
public:
    TelemetryDataPool(void);
    ~TelemetryDataPool(void);
    void SetLog(std::string id, uint32_t value);
    int GetLog(std::string id, TelemetryGeneralMetric& outLog);
    list<TelemetryGeneralMetric> GetAll(void);
    int GetNumEntries(void);

    static const int LIMIT_NUM_MAX_ENTRY = 100000;

private:
    tm _GetCurTime(void);

    std::map<std::string, TelemetryGeneralMetric> pool;
};

} // namespace pos
