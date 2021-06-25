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
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client_manager/telemetry_client_manager.h"

namespace pos
{
TelemetryClientManager::TelemetryClientManager(void)
: numClients(0)
{
}

TelemetryClientManager::~TelemetryClientManager(void)
{
}

int
TelemetryClientManager::RegisterClient(std::string name, TelemetryClient* client)
{
    auto ret = clientList.find(name);
    if (ret != clientList.end())
    {
        POS_TRACE_ERROR(EID(TELEMETRY_), "[Telemetry] error!! tried to add a client already registered:{}", name);
        return -1;
    }
    else
    {
        clientList.emplace(name, client);
        numClients++;
        POS_TRACE_ERROR(EID(TELEMETRY_), "[Telemetry] new client:{} is registered, numClients:{}", name, numClients);
        return 0;
    }
}

int
TelemetryClientManager::DeregisterClient(std::string name)
{
    auto ret = clientList.erase(name);
    if (ret == 0)
    {
        POS_TRACE_ERROR(EID(TELEMETRY_), "[Telemetry] error!! tried to erase client:{}, but it's not a telemetry client", name);
    }
    else
    {
        assert(ret == 1);
        POS_TRACE_ERROR(EID(TELEMETRY_), "[Telemetry] client:{} is removed", name);
    }
    numClients--;
    return 0;
}

void
TelemetryClientManager::StartTelemetryClient(std::string name)
{
    clientList[name]->StartLogging();
    POS_TRACE_ERROR(EID(TELEMETRY_), "[Telemetry] start logging for client:{}", name);
}

void
TelemetryClientManager::StopTelemetryClient(std::string name)
{
    clientList[name]->StopLogging();
    POS_TRACE_ERROR(EID(TELEMETRY_), "[Telemetry] stop logging for client:{}", name);
}

bool
TelemetryClientManager::IsTelemetryClientRunning(std::string name)
{
    return clientList[name]->IsRunning();
}

void
TelemetryClientManager::StartTelemetryClientAll(void)
{
    for(auto &p : clientList)
    {
        p.second->StartLogging();
    }
    POS_TRACE_ERROR(EID(TELEMETRY_), "[Telemetry] all started");
}

void
TelemetryClientManager::StopTelemetryClientAll(void)
{
    for(auto &p : clientList)
    {
        p.second->StopLogging();
    }
    POS_TRACE_ERROR(EID(TELEMETRY_), "[Telemetry] all stop");
}

int
TelemetryClientManager::CollectData(std::string name, std::string id, TelemetryLogEntry& outLog)
{
    auto it = clientList.find(name);
    if (it == clientList.end())
    {
        POS_TRACE_ERROR(EID(TELEMETRY_), "[Telemetry] Error!! Requested CollectData, there is no client id:{}", name);
        return -1;
    }
    else
    {
        return clientList[name]->CollectData(id, outLog);
    }
}

} // namespace pos