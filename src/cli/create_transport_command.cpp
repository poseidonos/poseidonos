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

#include "src/cli/create_transport_command.h"

#include <algorithm>
#include <string>

#include "src/cli/cli_event_code.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/include/nvmf_const.h"

namespace pos_cli
{
CreateTransportCommand::CreateTransportCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
CreateTransportCommand::~CreateTransportCommand(void)
{
}
// LCOV_EXCL_STOP

string
CreateTransportCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;

    int ret = 0;
    ret = _CreateTransport(doc);
    if (ret != SUCCESS)
    {
        return jFormat.MakeResponse(
            "CREATETRANSPORT", rid, ret,
            errorMessage, GetPosInfo());
    }

    return jFormat.MakeResponse(
        "CREATETRANSPORT", rid, SUCCESS,
        "Transport ( " + doc["param"]["transport_type"].get<string>() + " ) has been created.", GetPosInfo());
}

int
CreateTransportCommand::_CreateTransport(json& doc)
{
    SpdkRpcClient rpcClient;
    string trtype = doc["param"]["transport_type"].get<string>();

    _SetDefaultConfig();

    if (doc["param"].contains("buf_cache_size"))
    {
        bufCacheSize = doc["param"]["buf_cache_size"].get<uint32_t>();
    }
    if (doc["param"].contains("num_shared_buf"))
    {
        numSharedBuf = doc["param"]["num_shared_buf"].get<uint32_t>();
    }

    auto ret = rpcClient.TransportCreate(
        trtype,
        bufCacheSize,
        numSharedBuf,
        ioUnitSize);
    if (ret.first != SUCCESS)
    {
        errorMessage = "Failed to create transport. " + ret.second;
    }
    return ret.first;
}

void
CreateTransportCommand::_SetDefaultConfig()
{
    bufCacheSize = DEFAULT_BUF_CACHE_SIZE;
    numSharedBuf = DEFAULT_NUM_SHARED_BUF;
    ioUnitSize = DEFAULT_IO_UNIT_SIZE;
}
} // namespace pos_cli
