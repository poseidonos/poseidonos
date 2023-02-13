/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include <iostream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/bio/volume_io.h"
#include "src/io/general_io/translator.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Matcher;

namespace pos
{
class FakeVolumeIo : public VolumeIo
{
public:
    FakeVolumeIo(void);
    FakeVolumeIo(void *buffer, uint32_t unitCount, int arrayId);
    ~FakeVolumeIo(void);

    static void* operator new(std::size_t size) throw()
    {
        return ::operator new(size);
    }

    static void* operator new(std::size_t size, std::nothrow_t& n) throw()
    {
        return ::operator new(size, n);
    }
 
    static void* operator new(size_t size, void* ptr) throw()
    {
        return ::operator new(size, ptr);
    }

    static void operator delete(void* p)
    {
        return ::operator delete(p);
    }

    static void operator delete(void* p, std::size_t size) throw()
    {
        return ::operator delete(p, size);
    }

    static void operator delete(void* p, const std::nothrow_t& nt) throw()
    {
        return ::operator delete(p, nt);
    }

    void Init(void);

    virtual const StripeAddr& GetLsidEntry(void);
    virtual const StripeAddr& GetOldLsidEntry(void);
    virtual uint32_t GetOriginCore(void);
    virtual VolumeIoSmartPtr GetOriginVolumeIo(void);
    virtual uint64_t GetSectorRba(void);
    virtual StripeId GetUserLsid(void);
    virtual uint32_t GetVolumeId(void);
    virtual const VirtualBlkAddr& GetVsa(void);

    virtual bool IsPollingNecessary(void);

    virtual void SetLsidEntry(StripeAddr& lsidEntry);
    virtual void SetOldLsidEntry(StripeAddr& lsidEntry);
    virtual void SetSectorRba(uint64_t inputSectorRba);
    virtual void SetVolumeId(uint32_t inputVolumeId);
    virtual void SetVsa(VirtualBlkAddr&);
    virtual VolumeIoSmartPtr Split(uint32_t sectors, bool removalFromTail);

    void SetAllocated(void) { allocated = true; }
    bool GetAllocated(void) { return allocated; }

private:
    virtual bool _IsInvalidLsidEntry(StripeAddr& inputLsidEntry);
    void _ReflectSplit(UbioSmartPtr newUbio, uint32_t sectors, bool removalFromTail);

    bool allocated;
}; // namespace pos
}
