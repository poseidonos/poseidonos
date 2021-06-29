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

#include "src/cli/create_device_command.h"

#include "src/cli/cli_event_code.h"
#include "src/helper/spdk_rpc_client.h"

using namespace pos_cli;

CreateDeviceCommand::CreateDeviceCommand(void)
{
    _Init();
}

CreateDeviceCommand::~CreateDeviceCommand(void)
{
}

void
CreateDeviceCommand::_Init(void)
{
    errorMessage = "Failed to create buffer device. ";
}

string
CreateDeviceCommand::Execute(json& doc, string rid)
{
    _Init();

    JsonFormat jFormat;
    if (_CheckParamValidityAndSetType(doc) == false)
    {
        return jFormat.MakeResponse(
            "CREATEDEVICE", rid, -1, errorMessage, GetPosInfo());
    }

    if (type == "uram")
    {
        int ret = 0;
        ret = _CreateUramDevice(doc);
        if (ret != 0)
        {
            return jFormat.MakeResponse(
                "CREATEDEVICE", rid, -1, errorMessage, GetPosInfo());
        }
    }
    /*
    //TODO : PMEM Creation
    else if (type == "pmem")
    {
    }
*/

    return jFormat.MakeResponse(
        "CREATEDEVICE", rid, SUCCESS, "Device has been created", GetPosInfo());
}

int
CreateDeviceCommand::_CreateUramDevice(json& doc)
{
    SpdkRpcClient rpcClient;
    auto ret = rpcClient.BdevMallocCreate(
        doc["param"]["name"].get<string>(),
        doc["param"]["num_blocks"].get<uint32_t>(),
        doc["param"]["block_size"].get<uint32_t>());
    if (ret.first != 0)
    {
        errorMessage += ret.second;
    }
    return ret.first;
}

bool
CreateDeviceCommand::_CheckParamValidityAndSetType(json& doc)
{
    auto param = doc["param"];
    if (!param.contains("dev_type"))
    {
        errorMessage += "Device type must be included";
        return false;
    }
    type = param["dev_type"].get<string>();
    if (type != "uram" && type != "pmem")
    {
        errorMessage += "Device type must be uram or pmem";
        return false;
    }
    if (!param.contains("name"))
    {
        errorMessage += "Device name must be included";
        return false;
    }
    if (!param.contains("num_blocks"))
    {
        errorMessage += "Dumber of blocks must be included";
        return false;
    }
    if (!param.contains("block_size"))
    {
        errorMessage += "Block size must be included";
        return false;
    }

    return true;
}
