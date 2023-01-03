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

#include "src/wbt/set_write_bypass_wbt_command.h"

#include <string>

#include "src/array_mgmt/array_manager.h"

namespace pos
{
SetWriteBypassWbtCommand::SetWriteBypassWbtCommand(void)
: WbtCommand(SET_WRITE_BYPASS, "set_write_bypass")
{
}
// LCOV_EXCL_START
SetWriteBypassWbtCommand::~SetWriteBypassWbtCommand(void)
{
}
// LCOV_EXCL_STOP
int
SetWriteBypassWbtCommand::Execute(Args& argv, JsonElement& elem)
{
    std::string arrayName = argv["array"].get<std::string>();
    std::string value = argv["value"].get<std::string>();
    uint32_t numValue = std::stoul(value);
    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int event = EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
        POS_TRACE_WARN(event, "ArrayName is wrong. arrayName:{}", arrayName);
    }
    else
    {
        IArrayInfo* array = info->arrayInfo;
        array->SetNeedWriteBypass(numValue);
    }
    return 0;
}
} // namespace pos
