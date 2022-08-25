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

#ifndef SPDK_RPC_CLIENT_H_
#define SPDK_RPC_CLIENT_H_

#include <jsonrpccpp/client.h>

#include <string>
#include <utility>

#include "src/spdk_wrapper/caller/spdk_env_caller.h"

namespace pos
{
class SpdkRpcClient
{
public:
    SpdkRpcClient(SpdkEnvCaller* spdkEnvCaller = new SpdkEnvCaller());
    virtual ~SpdkRpcClient(void);
    std::pair<int, std::string> BdevMallocCreate(
        std::string name, uint32_t numBlocks, uint32_t blockSize, uint32_t numa);
    std::pair<int, std::string> SubsystemCreate(std::string subnqn, std::string sn, std::string mn, uint32_t max_namespaces, bool allow_any_host, bool ana_reporting);
    std::pair<int, std::string> SubsystemDelete(std::string subnqn);
    std::pair<int, std::string> SubsystemAddListener(std::string subnqn, std::string trtype, std::string adrfam, std::string traddr, std::string trsvcid);
    Json::Value SubsystemList(void);
    virtual std::pair<int, std::string> TransportCreate(std::string trtype, uint32_t bufCacheSize, uint32_t numSharedBuf, uint32_t ioUnitSize);

private:
    void _SetClient(void);

    jsonrpc::Client* client;
    jsonrpc::IClientConnector* connector;
    static const int SUCCESS = 0;

    SpdkEnvCaller* spdkEnvCaller;
};

} // namespace pos

#endif // SPDK_RPC_CLIENT_H_
