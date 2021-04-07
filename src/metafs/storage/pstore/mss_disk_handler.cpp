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

#include "mss_disk_handler.h"

#include "mss_on_disk.h"

#ifdef LEGACY_IOPATH

MssCompleteHandler::MssCompleteHandler(UbioSmartPtr input)
: ubio(input)
{
}

MssCompleteHandler::~MssCompleteHandler(void)
{
}

/**
 * Handler that will get return call from driver layer
 * Then notify comsumer of pstore about status.
 * Making it struct because parent is also struct. Which is param 
 * of  Ubio->endio
 */
bool
MssCompleteHandler::Execute(void)
{
    MssAioCbCxt* cbCxt = reinterpret_cast<MssAioCbCxt*>(ubio->ubioPrivate);

    MFS_TRACE_DEBUG("[IO done Notification] pageNum=", reinterpret_cast<MssAioData*>(cbCxt->GetAsycCbCxt())->metaLpn, ", addr=", ubio->address, ", buff=", ubio->GetBuffer(), ", error=", ubio->error);

    if (cbCxt != nullptr)
    {
        // For testing purposes
        cbCxt->SaveIOStatus(ubio->error);
        cbCxt->InvokeCallback();
    }

    // return false case for event re-try
    ubio = nullptr;
    return true;
}
#endif // #ifdef LEGACY_IOPATH
