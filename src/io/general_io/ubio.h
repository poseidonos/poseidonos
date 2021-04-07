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

#include <libaio.h>
#include <sys/uio.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "src/dump/dump_shared_ptr.h"
#include "src/dump/dump_shared_ptr.hpp"
#include "src/include/address_type.h"
#include "src/include/memory.h"
#include "src/io/general_io/data_buffer.h"
#include "src/scheduler/callback.h"
#if defined QOS_ENABLED_BE
#include "src/scheduler/event.h"
#endif

struct ibof_io;

namespace ibofos
{
enum class UbioDir
{
    Read,
    Write,
    WriteUncor,
    Deallocate,
    Abort,
#if defined NVMe_FLUSH_HANDLING
    Flush
#endif
};

struct DeviceLba;
class Event;
class Callback;
class UBlockDevice;
class Stripe;
class RequestContext;
class Ubio;

static const uint64_t MAX_PROCESSABLE_BLOCK_COUNT = 33;
static const uint64_t MAX_DUMP_ENTRIES_UBIO = 100;

using UbioSmartPtr = std::shared_ptr<Ubio>;

class Ubio : public DumpSharedPtr<Ubio*, static_cast<int>(DumpSharedPtrType::UBIO)>,
             public std::enable_shared_from_this<Ubio>
{
public:
    Ubio(void) = delete;
    Ubio(const Ubio& ubio);
    Ubio(void* buffer, uint32_t unitCount);
    virtual ~Ubio(void);

    static const uint32_t BYTES_PER_UNIT = SECTOR_SIZE;
    static const uint32_t UNITS_PER_BLOCK = BLOCK_SIZE / BYTES_PER_UNIT;

    UbioDir dir;
    void* ubioPrivate;

    UbioSmartPtr Split(uint32_t sectors, bool removalFromTail);
    void MarkDone(void);
    void* GetBuffer(uint32_t blockIndex = 0, uint32_t sectorOffset = 0) const;
    void* GetWholeBuffer(void) const;
    void WaitDone(void);

    void CompleteOrigin();
    void Complete(CallbackError error, bool executeCallback = true);
    void CompleteWithoutRecovery(CallbackError error, bool executeCallback = true);

    void SetPba(PhysicalBlkAddr& pbaInput);
    void FreeDataBuffer(void);
    void SetSyncMode(void);
    void IgnoreRecovery(void);
    void SetAsyncMode(void);
    void SetCallback(CallbackSmartPtr inputCallback);
    CallbackSmartPtr GetCallback(void);
    void ClearCallback(void);

    UBlockDevice* GetUBlock(void);
    ArrayDevice* GetDev(void);
    uint64_t GetLba(void);
    const PhysicalBlkAddr& GetPba(void);
    void SetRba(uint64_t inputSectorRba);
    uint64_t GetRba(void);
    bool IsRetry(void);
    void SetRetry(bool retry);
    uint32_t GetMemSize(void);
    virtual uint32_t GetOriginCore(void);
    void SetOriginUbio(UbioSmartPtr ubio);
    UbioSmartPtr GetOriginUbio(void);
    uint64_t GetSize(void);

    CallbackError GetError(void);
    void ResetError(void);

    bool CheckPbaSet(void);
    bool CheckValid();

#if defined QOS_ENABLED_BE
    uint64_t GetUbioSize(void);
    bool IsSyncMode(void);
    void SetEventType(BackendEvent event);
    BackendEvent GetEventType();
#endif
protected:
    static const uint32_t INVALID_CORE = UINT32_MAX;

#if defined QOS_ENABLED_BE
    BackendEvent eventIoType;
#endif
    void _ReflectSplit(UbioSmartPtr newUbio, uint32_t sectors,
        bool removalFromTail);

private:
    static const uint64_t INVALID_RBA = UINT64_MAX;
    static const uint64_t INVALID_LBA = UINT64_MAX;

    DataBuffer dataBuffer;
    CallbackSmartPtr callback;
    std::atomic<bool> syncDone;
    std::atomic<bool> sync;
    PhysicalBlkAddr pba;
    uint64_t sectorRba;
    bool retry;
    UbioSmartPtr origin;
    CallbackError error;
    bool referenceIncreased;
    bool recoveryIgnored;

    bool IsInvalidPba(PhysicalBlkAddr& inputPba);
    bool IsInvalidRba(uint64_t inputSectorRba);
    bool CheckRbaSet(void);
    bool CheckOriginUbioSet(void);
};

} // namespace ibofos
