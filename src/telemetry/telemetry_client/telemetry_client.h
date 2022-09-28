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
#include "src/telemetry/telemetry_client/grpc_global_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/telemetry/telemetry_config/telemetry_config.h"
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace pos
{
class GrpcGlobalPublisher;

class TelemetryClient : public ConfigObserver
{
public:
    explicit TelemetryClient(std::shared_ptr<grpc::Channel> channel_);
    TelemetryClient(void);
    virtual ~TelemetryClient(void);
    virtual int RegisterPublisher(TelemetryPublisher* tp);
    virtual int DeregisterPublisher(std::string name);
    virtual bool StartPublisher(std::string name);
    virtual bool StopPublisher(std::string name);
    virtual bool IsPublisherRunning(std::string name);
    virtual bool StartAllPublisher(void);
    virtual bool StopAllPublisher(void);
    virtual bool StartUsingDataPool(std::string name);
    virtual bool StopUsingDataPool(std::string name);
    virtual bool StartUsingDataPoolForAllPublisher(void);
    virtual bool StopUsingDataPoolForAllPublisher(void);
    virtual bool Notify(const std::string& key, const std::string& value) override;
    virtual bool IsPublisherRegistered(const std::string name);
    virtual bool LoadPublicationList(std::string filePath);
    virtual std::string GetPublicationList(void);
    virtual bool IsRunning(void);

private:
    std::string publicationListPath;
    std::map<std::string, TelemetryPublisher*> publisherList;
    GrpcGlobalPublisher* globalPublisher;
    std::atomic<uint64_t> publisherId;
    bool defaultEnable;
    bool isRunning;
};

using TelemetryClientSingleton = Singleton<TelemetryClient>;

} // namespace pos
