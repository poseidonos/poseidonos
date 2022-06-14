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

#include "src/cli/stop_pos_command.h"

#include <vector>

#include "src/array_mgmt/array_manager.h"
#include "src/cli/cli_event_code.h"
#include "src/cli/request_handler.h"
#include "src/logger/logger.h"
#include "src/mbr/mbr_info.h"

namespace pos_cli
{
StopPosCommand::StopPosCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
StopPosCommand::~StopPosCommand(void)
{
}
// LCOV_EXCL_STOP

string
StopPosCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    RequestHandler requestHandler;

    int ret = 0;
    std::vector<ArrayBootRecord> abrList;
    ret = ArrayManagerSingleton::Instance()->GetAbrList(abrList);

    if (ret == 0)
    {
        if (!abrList.empty())
        {
            int eventId = EID(MBR_ABR_GET_LIST_SUCCESS);
            POS_TRACE_DEBUG(eventId, "Found {} arrays from abr list", abrList.size());
            for (const auto& abr : abrList)
            {
                ComponentsInfo* CompInfo = ArrayMgr()->GetInfo(abr.arrayName);

                if (CompInfo == nullptr || CompInfo->arrayInfo == nullptr)
                {
                    continue;
                }
                IArrayInfo* arrayInfo = CompInfo->arrayInfo;

                if (arrayInfo->GetState() >= ArrayStateEnum::TRY_MOUNT)
                {
                    eventId = EID(POS_STOP_FAIULRE_MOUNTED_ARRAY_EXISTS);
                    POS_TRACE_ERROR(eventId,
                        "array:{}, state:{}",
                        abr.arrayName, arrayInfo->GetState().ToString());

                    return jFormat.MakeResponse("STOPPOS", rid, eventId, "", GetPosInfo());
                }
            }
        }
    }

    if (requestHandler.IsExit() == false)
    {
        requestHandler.SetExit(true);
        return jFormat.MakeResponse("STOPPOS", rid, SUCCESS,
            "POS will be terminated soon", GetPosInfo());
    }
    else
    {
        return jFormat.MakeResponse("STOPPOS", rid, SUCCESS,
            "POS is now terminating", GetPosInfo());
    }
}
}; // namespace pos_cli
