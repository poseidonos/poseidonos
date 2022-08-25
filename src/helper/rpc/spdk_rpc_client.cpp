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

#include "src/helper/rpc/spdk_rpc_client.h"

#include <jsoncpp/json/json.h>
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>
#include <jsonrpccpp/common/exception.h>

#include <string>
#include <algorithm>

#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

using namespace pos;
using namespace std;

SpdkRpcClient::SpdkRpcClient(SpdkEnvCaller* spdkEnvCaller)
: client(nullptr),
  connector(nullptr),
  spdkEnvCaller(spdkEnvCaller)
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
    if (spdkEnvCaller != nullptr)
    {
        delete spdkEnvCaller;
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
SpdkRpcClient::BdevMallocCreate(string name,
    uint32_t numBlocks,
    uint32_t blockSize,
    uint32_t numa)
{
    const int SUCCESS = 0;
    const string method = "bdev_malloc_create";

    Json::Value param;
    param["num_blocks"] = numBlocks;
    param["block_size"] = blockSize;
    param["name"] = name;
    param["numa"] = numa;

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

pair<int, string>
SpdkRpcClient::SubsystemCreate(string subnqn, string sn, string mn, uint32_t max_namespaces, bool allow_any_host, bool ana_reporting)
{
    const int SUCCESS = 0;
    const string method = "nvmf_create_subsystem";

    Json::Value param;
    param["nqn"] = subnqn;
    param["serial_number"] = sn;
    param["model_number"] = mn;
    param["allow_any_host"] = allow_any_host;
    param["ana_reporting"] = ana_reporting;
    param["max_namespaces"] = max_namespaces;

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

pair<int, string>
SpdkRpcClient::SubsystemDelete(string subnqn)
{
    const int SUCCESS = 0;
    const string method = "nvmf_delete_subsystem";

    Json::Value param;
    param["nqn"] = subnqn;

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

pair<int, std::string>
SpdkRpcClient::SubsystemAddListener(std::string subnqn, std::string trtype, std::string adrfam, std::string traddr, std::string trsvcid)
{
    const int SUCCESS = 0;
    const string method = "nvmf_subsystem_add_listener";

    Json::Value listen_address;
    listen_address["trtype"] = trtype;
    listen_address["adrfam"] = adrfam;
    listen_address["traddr"] = traddr;
    listen_address["trsvcid"] = trsvcid;

    Json::Value param;
    param["nqn"] = subnqn;
    param["listen_address"] = listen_address;

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

Json::Value
SpdkRpcClient::SubsystemList(void)
{
    const string method = "nvmf_get_subsystems";

    Json::Value param;

    Json::Value ret = client->CallMethod(method, param);

    return ret;
}

pair<int, std::string>
SpdkRpcClient::TransportCreate(std::string trtype, uint32_t bufCacheSize, uint32_t numSharedBuf, uint32_t ioUnitSize)
{
    const int SUCCESS = 0;
    const string method = "nvmf_create_transport";

    Json::Value param;
    param["trtype"] = trtype;

    std::for_each(trtype.begin(), trtype.end(), [](char& c)
    {
        c = tolower(c);
    });

    uint32_t coreCount = spdkEnvCaller->SpdkEnvGetCoreCount();
    uint32_t minSharedBuffers = coreCount * bufCacheSize;
    if (minSharedBuffers > numSharedBuf)
    {
        uint32_t coreCount = spdkEnvCaller->SpdkEnvGetCoreCount();
        uint32_t minSharedBuffers = coreCount * bufCacheSize;
        if (minSharedBuffers > numSharedBuf)
        {
            POS_EVENT_ID eventId =
                EID(IONVMF_TRANSPORT_NUM_SHARED_BUFFER_CHANGED);
            POS_TRACE_INFO(static_cast<int>(eventId),
            "Transport's num_shared_buffer size has changed from {} to {} due to reactor core number of system",
            numSharedBuf, minSharedBuffers);
            numSharedBuf = minSharedBuffers;
        }
        param["buf_cache_size"] = bufCacheSize;
        param["num_shared_buffers"] = numSharedBuf;
    }
    param["buf_cache_size"] = bufCacheSize;
    param["num_shared_buffers"] = numSharedBuf;
    param["io_unit_size"] = ioUnitSize;

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
