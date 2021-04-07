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

#include "mss.h"

#ifdef WITH_PSTORE_STORAGE
#include "mss_on_disk.h"
MssOnDisk pstore;
MetaStorageSubsystem* metaStorage = &pstore;
#else
#include "mss_ramdisk.h"
MssRamdisk vstore;
MetaStorageSubsystem* metaStorage = &vstore;
#endif

MetaStorageSubsystem::MetaStorageSubsystem(void)
: state(MetaSsState::Default)
{
}

MetaStorageSubsystem::~MetaStorageSubsystem(void)
{
    state = MetaSsState::Close;
}

bool
MetaStorageSubsystem::IsSuccess(IBOF_EVENT_ID ret)
{
    return (ret == IBOF_EVENT_ID::SUCCESS ? true : false);
}

bool
MetaStorageSubsystem::IsReady(void)
{
    return state == MetaSsState::Ready ? true : false;
}
void
MetaStorageSubsystem::_SetState(MetaSsState parmState)
{
    state = parmState;
}

IBOF_EVENT_ID
MetaStorageSubsystem::DoPageIO(MssOpcode opcode, MetaStorageType mediaType,
    MetaLpnType metaLpn, void* buffer, MetaLpnType numPages,
    uint32_t mpio_id, uint32_t tagId)
{
    switch (opcode)
    {
        case MssOpcode::Read:
        {
            return ReadPage(mediaType, metaLpn, buffer, numPages);
        }
        break;

        case MssOpcode::Write:
        {
            return WritePage(mediaType, metaLpn, buffer, numPages);
        }
        break;

        case MssOpcode::Trim:
        {
            return TrimFileData(mediaType, metaLpn, buffer, numPages);
        }
        break;

        default:
            assert(false);
    }
}

IBOF_EVENT_ID
MetaStorageSubsystem::DoPageIOAsync(MssOpcode opcode, MssAioCbCxt* cb)
{
    switch (opcode)
    {
        case MssOpcode::Read:
        {
            return ReadPageAsync(cb);
        }
        break;

        case MssOpcode::Write:
        {
            return WritePageAsync(cb);
        }
        break;

        case MssOpcode::Trim:
        {
            assert(false);
        }
        break;

        default:
            assert(false);
    }
}
