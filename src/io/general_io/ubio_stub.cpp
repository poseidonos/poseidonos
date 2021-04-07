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

#include <stdlib.h>

#include "src/io/general_io/ubio.h"

namespace ibofos
{
static PhysicalBlkAddr dummyPba;

Ubio::Ubio(void* buffer, uint32_t unitCount)
: dataBuffer(BYTES_PER_UNIT * unitCount, buffer)
{
}

Ubio::Ubio(const Ubio& ubio)
: dataBuffer(ubio.dataBuffer)
{
}

Ubio::~Ubio(void)
{
}
void
Ubio::CompleteWithoutRecovery(CallbackError errorCount, bool executeCallback)
{
}

UbioSmartPtr
Ubio::Split(uint32_t sectors, bool tail)
{
    return nullptr;
}

void
Ubio::MarkDone(void)
{
}

void*
Ubio::GetBuffer(uint32_t blockIndex, uint32_t sectorOffset) const
{
    return nullptr;
}

void*
Ubio::GetWholeBuffer(void) const
{
    return nullptr;
}

void
Ubio::WaitDone(void)
{
}

void
Ubio::Complete(CallbackError errorType, bool executeCallback)
{
}

void
Ubio::SetPba(PhysicalBlkAddr& pbaInput)
{
}

void
Ubio::FreeDataBuffer(void)
{
}

void
Ubio::SetSyncMode(void)
{
}

void
Ubio::SetAsyncMode(void)
{
}

UBlockDevice*
Ubio::GetUBlock(void)
{
    return nullptr;
}

uint64_t
Ubio::GetLba(void)
{
    return 0;
}

const PhysicalBlkAddr&
Ubio::GetPba(void)
{
    return dummyPba;
}

void
Ubio::SetRba(uint64_t inputSectorRba)
{
    sectorRba = inputSectorRba;
}

uint64_t
Ubio::GetRba(void)
{
    return sectorRba;
}

uint64_t
Ubio::GetSize(void)
{
    return dataBuffer.GetSize();
}

uint32_t
Ubio::GetOriginCore(void)
{
    return 0;
}

UbioSmartPtr
Ubio::GetOriginUbio(void)
{
    return origin;
}

void
Ubio::SetCallback(CallbackSmartPtr input)
{
}

} // namespace ibofos
