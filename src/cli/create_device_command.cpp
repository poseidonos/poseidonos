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

#include "src/cli/create_device_command.h"

#include "src/cli/cli_event_code.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

using namespace pos_cli;

CreateDeviceCommand::CreateDeviceCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
CreateDeviceCommand::~CreateDeviceCommand(void)
{
    if (spdkRpcClient != nullptr)
    {
        delete spdkRpcClient;
    }
}
// LCOV_EXCL_STOP

void
CreateDeviceCommand::Init(
    AffinityManager* affinityManager, SpdkRpcClient* spdkRpcClient)
{
    if (this->affinityManager == nullptr)
    {
        this->affinityManager = affinityManager;
    }

    if (this->spdkRpcClient == nullptr)
    {
        if (spdkRpcClient == nullptr)
        {
            this->spdkRpcClient = new SpdkRpcClient();
        }
        else
        {
            this->spdkRpcClient = spdkRpcClient;
        }
    }

    errorMessage = "Failed to create a device. ";
}


string
CreateDeviceCommand::Execute(json& doc, string rid)
{
    Init();

    JsonFormat jFormat;
    CreateDeviceParam param;

    if (!_ParseJsonToParam(param, doc))
    {
        return jFormat.MakeResponse(
            "CREATEDEVICE", rid, ERROR_CODE, errorMessage, GetPosInfo());
    }

    if (!_CheckParamValidity(param))
    {
        return jFormat.MakeResponse(
            "CREATEDEVICE", rid, ERROR_CODE, errorMessage, GetPosInfo());
    }

    int ret = _CreateUramDevice(param);
    if (ret != 0)
    {
        return jFormat.MakeResponse(
            "CREATEDEVICE", rid, ERROR_CODE, errorMessage, GetPosInfo());
    }

    return jFormat.MakeResponse(
        "CREATEDEVICE", rid, SUCCESS, "Device has been created", GetPosInfo());
}

int
CreateDeviceCommand::_CreateUramDevice(const CreateDeviceParam& param)
{
    auto ret = spdkRpcClient->BdevMallocCreate(
        param.name,
        param.numBlocks,
        param.blockSize,
        param.numa);

    if (ret.first != 0)
    {
        errorMessage += ret.second;
    }

    return ret.first;
}

bool
CreateDeviceCommand::_ParseJsonToParam(CreateDeviceParam& param, json& doc)
{
    auto jsonReq = doc["param"];

    if (!jsonReq.contains("devType"))
    {
        errorMessage += "Device type must be specified.";
        return false;
    }
    param.devType = jsonReq["devType"].get<string>();

    if (!jsonReq.contains("name"))
    {
        errorMessage += "Device name must be specified.";
        return false;
    }
    param.name = jsonReq["name"];

    if (!jsonReq.contains("numBlocks"))
    {
        errorMessage += "Number of blocks must be specified.";
        return false;
    }
    param.numBlocks = jsonReq["numBlocks"];

    if (!jsonReq.contains("blockSize"))
    {
        errorMessage += "Block size must be specified.";
        return false;
    }
    param.blockSize = jsonReq["blockSize"];

    if (jsonReq.contains("numa"))
    {
        param.numa =  jsonReq["numa"];
    }

    return true;
}

bool
CreateDeviceCommand::_CheckParamValidity(const CreateDeviceParam& param)
{
    if (param.devType != "uram")
    {
        errorMessage += "Not supported device type `" + param.devType + "`";
        return false;
    }

    uint32_t totalNumaCount = affinityManager->GetNumaCount();
    if (param.numa >= totalNumaCount)
    {
        errorMessage += "Invalid numa node. input= " + to_string(param.numa)
            + ", system count=" + to_string(totalNumaCount);
        return false;
    }

    return true;
}
