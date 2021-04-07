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

#ifdef LEGACY_IOPATH

#include "mfs_io_config.h"
#include "mfs_log.h"
#include "mss_status_callback.h"
#include "mss_utils.h"
#include "os_header.h"
#include "src/scheduler/event_argument.h"

using namespace ibofos;
struct MssCompleteHandler
{
public:
    bool operator()(EventArgument* eventArgument);
};

bool
MssCompleteHandler::operator()(EventArgument* eventArgument)
{
    Ubio* ubio = reinterpret_cast<Ubio*>(eventArgument->GetArgument(0));
    MssAioCbCxt* cbCxt = reinterpret_cast<MssAioCbCxt*>(ubio->ubioPrivate);

    MFS_TRACE_DEBUG("[IO done Notification] pageNum=", reinterpret_cast<MssAioData*>(cbCxt->GetAsycCbCxt())->metaLpn, ", addr=", ubio->address, ", buff=", ubio->GetBuffer(), ", error=", ubio->error);

    if (cbCxt != nullptr)
    {
        // For testing purposes
        cbCxt->SaveIOStatus(ubio->error);
        cbCxt->InvokeCallback();
    }

    delete ubio;
    return true;
}

#endif // LEGACY_IOPATH
