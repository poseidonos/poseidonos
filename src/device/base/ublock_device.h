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

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "src/bio/ubio.h"

namespace pos
{
static const uint32_t SSD_MAX_QUEUE_DEPTH = 1024;

class IOWorker;

enum class DeviceClass
{
    SYSTEM,
    ARRAY,
};

enum class DeviceType
{
    SSD,
    NVRAM,
    RAMDISK,
    ZSSD,
};

const int UNKNOWN_NUMA_NODE = -1;
class DeviceProperty
{
public:
    DeviceType type;
    DeviceClass cls;
    std::string name;
    uint64_t size;
    std::string mn;
    std::string sn;
    std::string bdf;
    int numa;

    std::string
    GetType(void)
    {
        switch (type)
        {
            case DeviceType::SSD:
                return "SSD";

            case DeviceType::NVRAM:
                return "NVRAM";

            case DeviceType::RAMDISK:
                return "RAMDISK";

            case DeviceType::ZSSD:
                return "ZSSD";

            default:
                return "";
        }
    }

    std::string
    GetClass(void)
    {
        switch (cls)
        {
            case DeviceClass::SYSTEM:
                return "SYSTEM";

            case DeviceClass::ARRAY:
                return "ARRAY";

            default:
                return "";
        }
    }
};

class DeviceContext;
class DeviceDriver;
class Ubio;

class UBlockDevice
{
public:
    explicit UBlockDevice(std::string name, uint64_t size,
        DeviceDriver* driverToUse);
    virtual ~UBlockDevice(void);

    virtual bool Open(void);
    virtual uint32_t Close(void);

    virtual int SubmitAsyncIO(UbioSmartPtr bio);
    virtual int CompleteIOs(void);
    virtual void* GetByteAddress(void);
    int Empty(void);

    virtual const char*
    GetName(void)
    {
        return property.name.c_str();
    }
    std::string
    GetName(void) const
    {
        return property.name;
    }
    virtual uint64_t
    GetSize(void)
    {
        return property.size;
    }
    virtual DeviceType
    GetType(void)
    {
        return property.type;
    }
    virtual std::string
    GetSN(void) const
    {
        return property.sn;
    }
    virtual std::string
    GetMN(void)
    {
        return property.mn;
    }
    virtual DeviceClass
    GetClass(void)
    {
        return property.cls;
    }
    virtual int
    GetNuma(void)
    {
        return property.numa;
    }
    virtual DeviceProperty
    GetProperty(void)
    {
        return property;
    }

    virtual void
    SetClass(DeviceClass cls)
    {
        property.cls = cls;
    }
    virtual bool IsAlive(void);
    virtual void ProfilePendingIoCount(uint32_t pendingIOCount);

    virtual void AddPendingErrorCount(uint32_t errorsToAdd = 1);
    virtual void SubtractPendingErrorCount(uint32_t errorsToSubtract = 1);
    virtual uint32_t GetPendingErrorCount(void);

    virtual void SetDedicatedIOWorker(IOWorker* ioWorker);
    virtual IOWorker* GetDedicatedIOWorker(void);

protected:
    virtual DeviceContext* _AllocateDeviceContext(void) = 0;
    virtual void _ReleaseDeviceContext(DeviceContext* deviceContextToRelease) = 0;
    DeviceContext* _GetDeviceContext(void);
    bool _OpenDeviceDriver(DeviceContext* deviceContextToOpen);
    bool _CloseDeviceDriver(DeviceContext* deviceContextToClose);
    uint32_t _Empty(DeviceContext* deviceContext);

    DeviceProperty property;

private:
    virtual bool _WrapupOpenDeviceSpecific(DeviceContext* devicecontext);

    bool _RegisterContextToCurrentCore(DeviceContext* devCtx);
    bool _RegisterThread(void);
    void _UnRegisterContextToCurrentCore(void);
    bool _IsCurrentCoreRegistered(void);
    bool _IsCoreRegistered(uint32_t coreId);

    static thread_local uint32_t currentThreadVirtualId;
    static std::atomic<uint32_t> lastVirtualId;
    static const uint32_t MAX_THREAD_COUNT = 128;
    static const uint32_t IO_WORKER_AID = 99;
    static const uint32_t ID_NOT_ALLOCATED = 0;

    DeviceDriver* driver;
    DeviceContext* contextArray[MAX_THREAD_COUNT];
    std::atomic<uint32_t> pendingErrorCount;
    std::atomic<bool> completeErrorAsSuccess;
    std::atomic<uint32_t> deviceContextCount;
    std::atomic<uint32_t> detachedFunctionProcessing;
    IOWorker* dedicatedIOWorker;
};

} // namespace pos
