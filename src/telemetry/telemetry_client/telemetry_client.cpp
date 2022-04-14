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
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"

#include <memory>

namespace pos
{

TelemetryClient::TelemetryClient(std::shared_ptr<grpc::Channel> channel_)
{
    globalPublisher = new GrpcGlobalPublisher(channel_);
    publisherId = 0;
    defaultEnable = false;
}

TelemetryClient::TelemetryClient(void)
: TelemetryClient(nullptr)
{
}

TelemetryClient::~TelemetryClient(void)
{
    delete globalPublisher;
}

int
TelemetryClient::RegisterPublisher(TelemetryPublisher* publisher)
{
    assert(publisher != nullptr);
    std::string name = publisher->GetName();
    name = to_string(publisherId.fetch_add(1)) + name;
    publisher->SetName(name);
    publisherList.emplace(name, publisher);
    publisher->SetGlobalPublisher(globalPublisher);
    if (defaultEnable == true)
    {
        publisher->StartPublishing();
    }
    POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] new publisher:{} is registered, numPublishers:{}, turnOn:{}", name, publisherList.size(), defaultEnable);
    return 0;
}

int
TelemetryClient::DeregisterPublisher(std::string name)
{
    auto ret = publisherList.erase(name);
    if (ret == 0)
    {
        POS_TRACE_ERROR(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] error!! tried to erase publisher:{}, but it's not registered", name);
    }
    else
    {
        POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] publisher:{} is removed, numPublishers:{}", name, publisherList.size());
    }
    return 0;
}

bool
TelemetryClient::StartPublisher(std::string name)
{
    publisherList[name]->StartPublishing();
    POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] start publisher:{}", name);
    return true;
}

bool
TelemetryClient::StopPublisher(std::string name)
{
    publisherList[name]->StopPublishing();
    POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] stop publisher:{}", name);
    return true;
}

bool
TelemetryClient::IsPublisherRunning(std::string name)
{
    return publisherList[name]->IsRunning();
}

bool
TelemetryClient::StartAllPublisher(void)
{
    POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] start all publishers");
    defaultEnable = true;
    for (auto &p : publisherList)
    {
        p.second->StartPublishing();
        POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] start publisher:{}", p.first);
    }
    return true;
}

bool
TelemetryClient::StopAllPublisher(void)
{
    POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] stop all publishers");
    defaultEnable = false;
    for (auto &p : publisherList)
    {
        p.second->StopPublishing();
        POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] stop publisher:{}", p.first);
    }
    return true;
}

bool
TelemetryClient::StartUsingDataPool(std::string name)
{
    publisherList[name]->StartUsingDataPool();
    POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] start Using DataPool Publisher:{}", name);
    return true;
}

bool
TelemetryClient::StopUsingDataPool(std::string name)
{
    publisherList[name]->StopUsingDataPool();
    POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] stop Using DataPool Publisher:{}", name);
    return true;
}

bool
TelemetryClient::StartUsingDataPoolForAllPublisher(void)
{
    POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] start using DataPool for all publishers");
    for (auto &p : publisherList)
    {
        p.second->StartUsingDataPool();
        POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] start using DataPool for publisher:{}", p.first);
    }
    return true;
}

bool
TelemetryClient::StopUsingDataPoolForAllPublisher(void)
{
    POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] stop using DataPool for all publishers");
    for (auto &p : publisherList)
    {
        p.second->StopUsingDataPool();
        POS_TRACE_INFO(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] stop using DataPool for publisher:{}", p.first);
    }
    return true;
}

bool
TelemetryClient::Notify(const std::string& key, const std::string& value)
{
    if (0 == key.compare("enabled"))
    {
        if (0 == value.compare("true"))
            StartAllPublisher();
        else
            StopAllPublisher();
    }

    return true;
}

bool
TelemetryClient::IsPublisherRegistered(const std::string name)
{
    auto ret = publisherList.find(name);
    return (ret != publisherList.end());
}

} // namespace pos
