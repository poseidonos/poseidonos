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

#include <libaio.h>
#include <sys/uio.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "src/bio/data_buffer.h"
#include "src/debug_lib/dump_shared_ptr.h"
#include "src/debug_lib/dump_shared_ptr.hpp"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event.h"
#include "src/include/address_type.h"
#include "src/include/io_error_type.h"
#include "src/include/memory.h"
#include "src/include/smart_ptr_type.h"

struct pos_io;

namespace pos
{
enum class UbioDir
{
    Read,
    Write,
    WriteUncor,
    Deallocate,
    Abort,
    NvmeCli,
    AdminPassTh,
    GetLogPage
};

struct DeviceLba;
class Stripe;
class RequestContext;

static const uint64_t MAX_PROCESSABLE_BLOCK_COUNT = 33;
static const uint64_t MAX_DUMP_ENTRIES_UBIO = 100;

class Ubio : public DumpSharedPtr<Ubio*, static_cast<int>(DumpSharedPtrType::UBIO)>,
             public std::enable_shared_from_this<Ubio>
{
public:
    Ubio(void) = delete;
    Ubio(const Ubio& ubio);
    Ubio(void* buffer, uint32_t unitCount, int arrayID);
    virtual ~Ubio(void);

    static const uint32_t BYTES_PER_UNIT = SECTOR_SIZE;
    static const uint32_t UNITS_PER_BLOCK = BLOCK_SIZE / BYTES_PER_UNIT;

    UbioDir dir;
    void* ubioPrivate;

    UbioSmartPtr Split(uint32_t sectors, bool removalFromTail);
    void MarkDone(void);
    void* GetBuffer(uint32_t blockIndex = 0, uint32_t sectorOffset = 0) const;
    virtual void* GetWholeBuffer(void) const;
    virtual void WaitDone(void);

    virtual void Complete(IOErrorType error);
    virtual void ClearOrigin(void);

    void SetPba(PhysicalBlkAddr& pbaInput);
    void FreeDataBuffer(void);
    virtual void SetSyncMode(void);
    void SetAsyncMode(void);
    void SetCallback(CallbackSmartPtr inputCallback);
    virtual CallbackSmartPtr GetCallback(void);
    virtual void ClearCallback(void);

    virtual UBlockDevice* GetUBlock(void);
    virtual IArrayDevice* GetArrayDev(void);
    uint64_t GetLba(void);
    const PhysicalBlkAddr GetPba(void);
    virtual bool IsRetry(void);
    void SetRetry(bool retry);
    uint32_t GetMemSize(void);
    virtual uint32_t GetOriginCore(void);
    virtual void SetOriginCore(uint32_t originCore);
    void SetOriginUbio(UbioSmartPtr ubio);
    virtual UbioSmartPtr GetOriginUbio(void);
    virtual uint64_t GetSize(void);

    IOErrorType GetError(void);
    virtual void SetError(IOErrorType inputErrorType);
    void ResetError(void);

    bool CheckPbaSet(void);
    virtual bool CheckRecoveryAllowed(void);

    virtual bool IsSyncMode(void);

    void SetLba(uint64_t lba);
    void SetUblock(UblockSharedPtr uBlock);
    virtual bool NeedRecovery(void);

    virtual int GetArrayId(void);
    void SetEventType(BackendEvent event);
    BackendEvent GetEventType(void);

protected:
    BackendEvent eventIoType;
    void _ReflectSplit(UbioSmartPtr newUbio, uint32_t sectors,
        bool removalFromTail);

private:
    static const uint64_t INVALID_LBA = UINT64_MAX;

    DataBuffer dataBuffer;
    CallbackSmartPtr callback;
    std::atomic<bool> syncDone;
    std::atomic<bool> sync;
    bool retry;
    UbioSmartPtr origin;
    IOErrorType error;
    uint64_t lba;
    UblockSharedPtr uBlock;
    IArrayDevice* arrayDev;
    int arrayId;
    std::string arrayName;
    uint32_t originCore;

    bool CheckOriginUbioSet(void);
    void Advance(uint32_t sectors);
    void Retreat(uint32_t sectors);
};

} // namespace pos
