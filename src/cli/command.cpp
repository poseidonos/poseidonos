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

#include "src/cli/command.h"

#include "src/sys_info/space_info.h"

#include "src/array_mgmt/array_manager.h"
namespace pos_cli
{
Command::Command(void)
{
}

Command::~Command(void)
{
}

JsonElement
GetPosInfo(string name)
{
    JsonElement data(name);

    PosInfo info;
    // TODO: what is this??
    string arrayName = "";
    IArrayInfo* arrayInfo = ArrayMgr::Instance()->GetArrayInfo(arrayName);
    if (arrayInfo != nullptr)
    {
        info.state = arrayInfo->GetState().ToString();
        info.rebuildingProgress = arrayInfo->GetRebuildingProgress();
        info.totalCapacity = SpaceInfo::SystemCapacity(arrayName);
        info.usedCapacity = SpaceInfo::Used(arrayName);
    }

    data.SetAttribute(JsonAttribute("state", "\"" + info.state + "\""));
    data.SetAttribute(JsonAttribute("rebuildingProgress", "\"" + to_string(info.rebuildingProgress) + "\""));
    data.SetAttribute(JsonAttribute("capacity", to_string(info.totalCapacity)));
    data.SetAttribute(JsonAttribute("used", to_string(info.usedCapacity)));

    return data;
}
}; // namespace pos_cli
