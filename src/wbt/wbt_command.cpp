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

#include "wbt_command.h"

namespace pos
{

WbtCommand::WbtCommand(WbtCommandOpcode opcodeToUse, const std::string &commandName)
:   opcode(opcodeToUse),
    name(commandName)
{
}
// LCOV_EXCL_START
WbtCommand::~WbtCommand(void)
{
}
// LCOV_EXCL_STOP
const std::string &
WbtCommand::GetCommandName(void)
{
    return name;
}

WbtCommandOpcode
WbtCommand::GetOpcode(void)
{
    return opcode;
}

std::string
WbtCommand::_GetParameter(Args& argv, const char* paramToSearch)
{
    if (argv.contains(paramToSearch))
        return argv[paramToSearch].get<std::string>();

    std::string emptyString;

    return emptyString;
}

uint32_t
WbtCommand::_ConvertHexStringToUINT32(std::string& value)
{
    uint64_t startOffsetOfRealValue = value.find("0x");
    if (std::string::npos != startOffsetOfRealValue)
    {
        return std::stoul(value.substr(startOffsetOfRealValue), nullptr, 16);
    }

    return std::stoul(value);
}

} // namespace pos
