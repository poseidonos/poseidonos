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

#ifndef DEBUG_INFO_H_
#define DEBUG_INFO_H_

#include "src/debug/debug_info_updater.h"

namespace pos
{
class AffinityManager;
class AllocatorService;
class ArrayManager;
class CommandTimeoutHandler;
class ConfigManager;
class DebugInfo;
class DeviceManager;
class DumpManager;
class EventScheduler;
class FlushCmdManager;
class FlushCount;
class GarbageCollector;
class IODispatcher;
class IODispatcherSubmission;
class IOSubmitHandlerCount;
class Logger;
class MapperService;
class MbrManager;
class MockDeviceDriver;
class QosManager;
class RBAStateService;
class Reporter;
class SignalHandler;
class Spdk;
class UnvmeDrv;
class UramDrv;
class VersionProvider;
class VolumeEventPublisher;
class VolumeService;
class MetaFsService;
class NvmfTarget;
class TelemetryClient;
class TelemetryManagerService;
class TelemetryConfig;
class MemoryManager;
class PosReplicatorManager;
class ResourceChecker;
class IoTimeoutChecker;

extern DebugInfo* debugInfo;

class DebugInfo : public DebugInfoUpdater
{
public:
    DebugInfo(void);
    ~DebugInfo(void) override;

    void Update(void) override;

private:
    AffinityManager* affinityManager;
    AllocatorService* allocatorService;
    ArrayManager* arrayManager;
    CommandTimeoutHandler* commandTimeoutHandler;
    ConfigManager* configManager;
    DeviceManager* deviceManager;
    DumpManager* dumpManager;
    EventScheduler* eventScheduler;
    FlushCmdManager *flushCmdManager;
    FlushCount* flushCount;
    GarbageCollector* garbageCollector;
    IODispatcher* ioDispatcher;
    IODispatcherSubmission* ioDispatcherSubmission;
    IOSubmitHandlerCount* ioSubmitHandlerCount;
    Logger* logger;
    MapperService* mapperService;
    MetaFsService* metaFsService;
    QosManager *qosManager;
    RBAStateService* rbaStateService;
    Reporter* reporter;
    SignalHandler* signalHandler;
    Spdk* spdk;
    UnvmeDrv* unvmeDrv;
    UramDrv* uramDrv;
    VersionProvider *versionProvider;
    VolumeEventPublisher* volumeEventPublisher;
    VolumeService* volumeService;
    NvmfTarget* nvmfTarget;
    TelemetryManagerService* telemetryManagerService;
    TelemetryClient* telemetryClient;
    TelemetryConfig* telemetryConfig;
    MemoryManager* memoryManager;
    PosReplicatorManager* posReplicatorManager;
    ResourceChecker* resourceChecker;
    IoTimeoutChecker* ioTimeoutChecker;
};

} // namespace pos
#endif // DEBUG_INFO_H_
