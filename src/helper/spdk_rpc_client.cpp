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

#include "src/helper/spdk_rpc_client.h"

#include <jsoncpp/json/json.h>
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>
#include <jsonrpccpp/common/exception.h>

#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

using namespace pos;
using namespace std;

SpdkRpcClient::SpdkRpcClient(void)
: client(nullptr),
  connector(nullptr)
{
    _SetClient();
}

SpdkRpcClient::~SpdkRpcClient(void)
{
    if (connector != nullptr)
    {
        delete connector;
    }
    if (client != nullptr)
    {
        delete client;
    }
}

void
SpdkRpcClient::_SetClient(void)
{
    const string defaultSpdkSocketPath = "/var/tmp/spdk.sock";
    jsonrpc::IClientConnector* connector =
        new jsonrpc::UnixDomainSocketClient(defaultSpdkSocketPath);
    client = new jsonrpc::Client(*connector);
}

pair<int, string>
SpdkRpcClient::BdevMallocCreate(string name, uint32_t numBlocks, uint32_t blockSize)
{
    const int SUCCESS = 0;
    const string method = "bdev_malloc_create";

    Json::Value param;
    param["num_blocks"] = numBlocks;
    param["block_size"] = blockSize;
    param["name"] = name;

    Json::Value ret;
    try
    {
        client->CallMethod(method, param);
    }
    catch (jsonrpc::JsonRpcException const& e)
    {
        return make_pair(e.GetCode(), e.GetMessage());
    }

    return make_pair(SUCCESS, "");
}
