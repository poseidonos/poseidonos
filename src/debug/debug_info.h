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

#ifndef DEBUG_INFO_H_
#define DEBUG_INFO_H_

class MetaVolMgrClass;
namespace ibofos
{
class AffinityManager;
class Allocator;
class Array;
class CommandTimeoutHandler;
class ConfigManager;
class DebugInfo;
class DeviceManager;
class DumpManager;
class EventScheduler;
#if defined NVMe_FLUSH_HANDLING
class FlushCmdManager;
#endif
class GarbageCollector;
class IODispatcher;
class JournalManager;
class Logger;
class Mapper;
class MbrManager;
class MockDeviceDriver;
class QosManager;
class RBAStateManager;
class Reporter;
class Spdk;
class StateManager;
class StatePolicy;
class UnvmeDrv;
class UnvramDrv;
class VersionProvider;
class VolumeEventPublisher;
class VolumeManager;

extern DebugInfo* debugInfo;

class DebugInfo
{
public:
    DebugInfo(void);
    void UpdateModulePointer(void);

private:
    AffinityManager* affinityManager;
    Allocator* allocator;
    Array* array;
    CommandTimeoutHandler* commandTimeoutHandler;
    ConfigManager* configManager;
    DeviceManager* deviceManager;
    DumpManager* dumpManager;
    EventScheduler* eventScheduler;
#if defined NVMe_FLUSH_HANDLING
    FlushCmdManager *flushCmdManager;
#endif
    GarbageCollector* garbageCollector;
    IODispatcher* ioDispatcher;
    JournalManager* journalManager;
    Logger* logger;
    Mapper* mapper;
    MbrManager* mbrManager;
    MetaVolMgrClass* metaVolManager;
    QosManager *qosManager;
    RBAStateManager* rbaStateManager;
    Reporter* reporter;
    Spdk* spdk;
    StateManager* stateManager;
    StatePolicy* statePolicy;
    UnvmeDrv* unvmeDrv;
    UnvramDrv* unvramDrv;
    VersionProvider *versionProvider;
    VolumeEventPublisher* volumeEventPublisher;
    VolumeManager* volumeManager;
};

} // namespace ibofos
#endif // DEBUG_INFO_H_
