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

#ifdef _ADMIN_ENABLED
#pragma once
#include <atomic>
#include <mutex>
#include <string>

#include "src/include/memory.h"
#include "src/lib/singleton.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/async_context.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/meta_file_intf/meta_file_intf.h"
#ifndef IBOF_CONFIG_USE_MOCK_FS
#include "src/metafs/metafs_file_intf.h"
#endif
#include "src/volume/volume_list.h"
using namespace std;

namespace pos
{
static const uint64_t BLOCK_SIZE_SMART = pos::BLOCK_SIZE;
struct SmartLogEntry
{
    std::atomic<std::uint64_t> volId;
    std::atomic<std::uint64_t> hostReadCommands;
    std::atomic<std::uint64_t> hostWriteCommands;
    std::atomic<std::uint64_t> bytesWritten;
    std::atomic<std::uint64_t> bytesRead;
};

class LogPageFlushIoCtx : public AsyncMetaFileIoCtx
{
public:
    int mpageNum;
};

class SmartLogMgr
{
public:
    SmartLogMgr(void);
    ~SmartLogMgr(void);

    void Init(MetaFileIntf* metaFileIntf);
    int StoreLogData(void);
    int LoadLogData(void);
    void IncreaseReadCmds(uint32_t volId);
    void IncreaseWriteCmds(uint32_t volId);
    uint64_t GetWriteCmds(uint32_t volId);
    uint64_t GetReadCmds(uint32_t volId);

    void IncreaseReadBytes(uint64_t blkCnt, uint32_t volId);
    void IncreaseWriteBytes(uint64_t blkCnt, uint32_t volId);
    uint64_t GetWriteBytes(uint32_t volId);
    uint64_t GetReadBytes(uint32_t volId);
    // Meta file
    int CreateSmartLogFile(void);
    int OpenFile(void);
    bool IsFileOpened(void);
    int CloseFile(void);
    int DeleteSmartLogFile(void);
    void CompleteSmartLogIo(AsyncMetaFileIoCtx* ctx);

private:
    // Meta File
    int _DoMfsOperation(int Direction);
    MetaFileIntf* smartLogFile;
    std::string fileName;
    // SmartLogMetafs *smartLogMetafs;
    struct SmartLogEntry logPage[MAX_VOLUME_COUNT];
    bool loaded;
    int ioError = 0;
    int ioDirection = 0;
};
using SmartLogMgrSingleton = Singleton<SmartLogMgr>;
} // namespace pos
#endif
