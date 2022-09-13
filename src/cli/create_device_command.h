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

#ifndef CREATE_DEVICE_COMMAND_H_
#define CREATE_DEVICE_COMMAND_H_

#include <string>

#include "src/cli/command.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/include/pos_event_id.h"

using namespace pos;

namespace pos
{
class SpdkRpcClient;
} // namespace pos

namespace pos_cli
{

struct CreateDeviceParam
{
    string name;
    string devType;
    uint32_t numBlocks = 0;
    uint32_t blockSize = 0;
    uint32_t numa = 0;
};

class CreateDeviceCommand : public Command
{
public:
    CreateDeviceCommand(void);
    ~CreateDeviceCommand(void) override;

    void Init(
        AffinityManager* affinityManager = AffinityManagerSingleton::Instance(),
        SpdkRpcClient* spdkRpcClient = nullptr);
    string Execute(json& doc, string rid) override;

private:
    bool _ParseJsonToParam(CreateDeviceParam& param, json& doc);
    bool _CheckParamValidity(const CreateDeviceParam& param);
    int _CreateUramDevice(const CreateDeviceParam& param);

    string errorMessage;
    const int ERROR_CODE = static_cast<int>(EID(CLI_CREATE_DEVICE_FAILURE));

    AffinityManager* affinityManager = nullptr;
    SpdkRpcClient* spdkRpcClient = nullptr;
};

} // namespace pos_cli

#endif // CREATE_DEVICE_COMMAND_H_
