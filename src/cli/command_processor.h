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

#pragma once

#include "proto/generated/cpp/cli.grpc.pb.h"
#include "proto/generated/cpp/cli.pb.h"
#include <string>

using grpc::Status;
using grpc::StatusCode;
using grpc_cli::SystemInfoRequest;
using grpc_cli::SystemInfoResponse;
using grpc_cli::SystemStopRequest;
using grpc_cli::SystemStopResponse;
using grpc_cli::GetSystemPropertyRequest;
using grpc_cli::GetSystemPropertyResponse;
using grpc_cli::SetSystemPropertyRequest;
using grpc_cli::SetSystemPropertyResponse;

class CommandProcessor
{
public:
    CommandProcessor(void);
    ~CommandProcessor(void);
    void FillHeader(const SystemInfoRequest* request, SystemInfoResponse* reply);
    Status ExecuteSystemInfoCommand(const SystemInfoRequest* request, SystemInfoResponse* reply);
    Status ExecuteSystemStopCommand(const SystemStopRequest* request, SystemStopResponse* reply);
    Status ExecuteGetSystemPropertyCommand(const GetSystemPropertyRequest* request, GetSystemPropertyResponse* reply);
    Status ExecuteSetSystemPropertyCommand(const SetSystemPropertyRequest* request, SetSystemPropertyResponse* reply);

private:
    bool _isPosTerminating;
    bool _IsPosTerminating(void) { return _isPosTerminating; }
    void _SetPosTerminating(bool input) { _isPosTerminating = input; }
    std::string _GetRebuildImpactString(uint8_t impact);
};
