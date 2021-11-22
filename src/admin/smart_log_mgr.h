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
#include <atomic>
#include <mutex>
#include <string>

#include "src/include/array_mgmt_policy.h"
#include "src/include/memory.h"
#include "src/lib/singleton.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/volume/volume_list.h"

using namespace std;

namespace pos
{
static const uint64_t BLOCK_SIZE_SMART = pos::BLOCK_SIZE;
static const uint32_t MAX_ARRAYS = ArrayMgmtPolicy::MAX_ARRAY_CNT;

struct SmartLogEntry
{
    std::atomic<std::uint64_t> volId;
    std::atomic<std::uint64_t> hostReadCommands;
    std::atomic<std::uint64_t> hostWriteCommands;
    std::atomic<std::uint64_t> bytesWritten;
    std::atomic<std::uint64_t> bytesRead;
};

class SmartLogMgr
{
public:
    SmartLogMgr(void);
    explicit SmartLogMgr(ConfigManager* configManager);
    virtual ~SmartLogMgr(void);

    virtual void Init(void);
    virtual bool GetSmartLogEnabled(void);
    void IncreaseReadCmds(uint32_t volId, uint32_t arrayId);
    void IncreaseWriteCmds(uint32_t volId, uint32_t arrayId);
    uint64_t GetWriteCmds(uint32_t volId, uint32_t arrayId);
    uint64_t GetReadCmds(uint32_t volId, uint32_t arrayId);

    void IncreaseReadBytes(uint64_t blkCnt, uint32_t volId, uint32_t arrayId);
    void IncreaseWriteBytes(uint64_t blkCnt, uint32_t volId, uint32_t arrayId);
    virtual uint64_t GetWriteBytes(uint32_t volId, uint32_t arrayId);
    virtual uint64_t GetReadBytes(uint32_t volId, uint32_t arrayId);
    void* GetLogPages(uint32_t arrayId);

private:
    struct SmartLogEntry logPage[MAX_ARRAYS][MAX_VOLUME_COUNT];
    bool smartLogEnable = false;
    ConfigManager* configManager = nullptr;
};
using SmartLogMgrSingleton = Singleton<SmartLogMgr>;
} // namespace pos
