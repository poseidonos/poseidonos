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

#include "src/admin/smart_log_mgr.h"

#include <spdk/nvme_spec.h>

#include "src/include/pos_event_id.hpp"

namespace pos
{
SmartLogMgr::SmartLogMgr(void)
{
    configManager = ConfigManagerSingleton::Instance();
}
SmartLogMgr::SmartLogMgr(ConfigManager* configMgr)
{
    configManager = configMgr;
}
SmartLogMgr::~SmartLogMgr(void)
{
}

void
SmartLogMgr::Init(void)
{
    bool enabled = false;
    smartLogEnable = false;
    int ret = configManager->GetValue("admin", "smart_log_page", &enabled, CONFIG_TYPE_BOOL);
    if (ret == EID(SUCCESS))
    {
        smartLogEnable = enabled;
    }
    memset(logPage, 0, MAX_VOLUME_COUNT * sizeof(struct SmartLogEntry));
}

bool
SmartLogMgr::GetSmartLogEnabled(void)
{
    return smartLogEnable;
}

void*
SmartLogMgr::GetLogPages(uint32_t arrayId)
{
    return (void*)logPage[arrayId];
}

void
SmartLogMgr::IncreaseReadCmds(uint32_t volId, uint32_t arrayId)
{
    if (GetSmartLogEnabled() == false)
    {
        return;
    }
    logPage[arrayId][volId].hostReadCommands = logPage[arrayId][volId].hostReadCommands + 1;
    return;
}

void
SmartLogMgr::IncreaseWriteCmds(uint32_t volId, uint32_t arrayId)
{
    if (GetSmartLogEnabled() == false)
    {
        return;
    }
    logPage[arrayId][volId].hostWriteCommands = logPage[arrayId][volId].hostWriteCommands + 1;
    return;
}

uint64_t
SmartLogMgr::GetWriteCmds(uint32_t volId, uint32_t arrayId)
{
    return logPage[arrayId][volId].hostWriteCommands;
}

uint64_t
SmartLogMgr::GetReadCmds(uint32_t volId, uint32_t arrayId)
{
    return logPage[arrayId][volId].hostReadCommands;
}

void
SmartLogMgr::IncreaseReadBytes(uint64_t blkCnt, uint32_t volId, uint32_t arrayId)
{
    if (GetSmartLogEnabled() == false)
    {
        return;
    }
    logPage[arrayId][volId].bytesRead = logPage[arrayId][volId].bytesRead + blkCnt * BLOCK_SIZE_SMART;
    return;
}

void
SmartLogMgr::IncreaseWriteBytes(uint64_t blkCnt, uint32_t volId, uint32_t arrayId)
{
    if (GetSmartLogEnabled() == false)
    {
        return;
    }
    logPage[arrayId][volId].bytesWritten = logPage[arrayId][volId].bytesWritten + blkCnt * BLOCK_SIZE_SMART;
    return;
}

uint64_t
SmartLogMgr::GetReadBytes(uint32_t volId, uint32_t arrayId)
{
    return logPage[arrayId][volId].bytesRead;
}

uint64_t
SmartLogMgr::GetWriteBytes(uint32_t volId, uint32_t arrayId)
{
    return logPage[arrayId][volId].bytesWritten;
}

} // namespace pos
