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
#include <string>

#include "src/admin/smart_log_mgr.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/meta_file_intf/async_context.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/meta_file_intf/meta_file_intf.h"
#include "src/volume/volume_list.h"

using namespace std;

namespace pos
{
class LogPageFlushIoCtx : public AsyncMetaFileIoCtx
{
public:
    int mpageNum;
};

class MetaIoDoneChecker
{
public:
    MetaIoDoneChecker(void)
    : ioDone(false)
    {
    }
    virtual ~MetaIoDoneChecker(void)
    {
    }
    virtual void SetReady(void)
    {
        ioDone = false;
    }
    virtual void SetDone(void)
    {
        ioDone = true;
    }
    virtual bool IsDone(void)
    {
        return ioDone;
    }

private:
    std::atomic<bool> ioDone;
};

class SmartLogMetaIo : public IMountSequence
{
public:
    SmartLogMetaIo(uint32_t arrayIndex, SmartLogMgr* smartLogMgr);
    // only for test
    SmartLogMetaIo(uint32_t arrayIndex, SmartLogMgr* smartLogMgr, MetaFileIntf* metaFile, MetaIoDoneChecker* ioDone);
    virtual ~SmartLogMetaIo(void);
    virtual int Init(void) override;
    virtual void Dispose(void) override;
    virtual void Shutdown(void) override;
    virtual void Flush(void) override;
    void DeleteAsyncIoCtx(AsyncMetaFileIoCtx* ctx);
private:
    // Meta File
    int _CreateFile(void);
    int _StoreLogData(void);
    int _LoadLogData(void);
    int _OpenFile(void);
    int _CloseFile(void);
    void _CompleteSmartLogIo(AsyncMetaFileIoCtx* ctx);
    int _DoMfsOperation(int Direction);
    void _SetCheckerReady(void);
    void _SetCheckerDone(void);
    void _WaitForCheckerDone(void);

    bool loaded;
    std::string fileName;
    MetaFileIntf* smartLogFile;
    uint32_t arrayId;
    int ioError = 0;
    SmartLogMgr* smartLogMgr;
    MetaIoDoneChecker* fileIoDone;
    bool rocksDbEnabled;
};
} // namespace pos
