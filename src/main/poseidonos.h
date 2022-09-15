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
#include <thread>
#include <string>
#include "src/debug/debug_info.h"
#include "src/master_context/config_manager.h"
#include "src/master_context/version_provider.h"
#include "src/trace/trace_exporter.h"

namespace pos
{
class IoRecoveryEventFactory;
class TelemetryAirDelegator;
class TelemetryPublisher;
class SignalHandler;

class Poseidonos
{
public:
    int Init(int argc, char** argv);
    void Run(void);
    void Terminate(void);
    // This function should be private. But being public for only UT
    int _InitTraceExporter(char* procFullName,
                            ConfigManager *cm,
                            VersionProvider *vp,
                            TraceExporter *te);

private:
    void _InitDebugInfo(void);
    void _InitSignalHandler(void);
    void _InitSpdk(int argc, char** argv);

    void _InitAffinity(void);
    void _InitIOInterface(void);
    void _LoadVersion(void);

    void _InitAIR(void);
    void _InitMemoryChecker(void);
    void _InitResourceChecker(void);
    void _InitReplicatorManager(void);
    void _SetPerfImpact(void);
    int _LoadConfiguration(void);
    void _RunCLIService(void);
    void _SetupThreadModel(void);

    static const uint32_t EVENT_THREAD_CORE_RATIO = 1;

    IoRecoveryEventFactory* ioRecoveryEventFactory = nullptr;
    TelemetryAirDelegator* telemetryAirDelegator = nullptr;
    TelemetryPublisher* telemtryPublisherForAir = nullptr;
    SignalHandler* signalHandler = nullptr;

    std::thread *GrpcCliServerThread;
};
} // namespace pos
