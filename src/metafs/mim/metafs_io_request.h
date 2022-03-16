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

#include <string>
#include "meta_storage_specific.h"
#include "metafs_common.h"
#include "metafs_mutex.h"
#include "metafs_spinlock.h"
#include "os_header.h"
#include "src/metafs/include/meta_file_context.h"
#include "src/bio/volume_io.h"
#include "src/metafs/include/meta_file_extent.h"
#include "src/metafs/common/metafs_stopwatch.h"
#include "src/meta_file_intf/meta_file_include.h"

namespace pos
{
enum class MetaIoRequestType
{
    Write = 0,
    Read,

    Max
};

enum class MetaIoOpcode
{
    Write = 0,
    Read,

    // Delete,

    Max,
};

enum class MetaIoRange
{
    FullFileIo = 0,
    ByteRangeFileIo,

    Max
};

enum class MetaIoMode
{
    Sync = 0,
    Async = 1,

    Max
};

enum class IoRequestStage
{
    Create,
    Enqueue,
    Dequeue,
    Complete,
    Count
};

// basic io req. info. given by caller
class MetaFsIoRequest : public MetaFsRequestBase, public MetaFsStopwatch<IoRequestStage>
{
public:
    MetaFsIoRequest(void);
    virtual ~MetaFsIoRequest(void);

    virtual void CopyUserReqMsg(const MetaFsIoRequest& req);
    virtual bool IsValid(void);
    virtual bool IsSyncIO(void);
    virtual bool IsIoCompleted(void);
    virtual bool GetError(void);
    virtual void SuspendUntilIoCompletion(void);
    virtual void NotifyIoCompletionToClient(void);
    virtual void SetError(bool err);
    virtual void SetRetryFlag(void);
    virtual bool GetRetryFlag(void)
    {
        return retryFlag;
    }
    virtual std::string GetLogString(void) const;

    MetaIoRequestType reqType;
    MetaIoMode ioMode;
    bool isFullFileIo;
    FileDescriptorType fd;
    int arrayId;
    FileBufType buf;
    FileSizeType byteOffsetInFile;
    FileSizeType byteSize;
    MetaStorageType targetMediaType;
    void* aiocb;

    uint32_t tagId;
    MetaLpnType baseMetaLpn;
    MetaFileExtent* extents;
    int extentsCount;
    MetaFsIoRequest* originalMsg;
    int requestCount;
    MetaFileContext* fileCtx;
    RequestPriority priority;

private:
    bool retryFlag;
    std::atomic<bool> ioDone;
    std::atomic<bool> error;

    MetaFsSpinLock cbLock;

    static const FileBufType INVALID_BUF;
    static const FileSizeType INVALID_BYTE_OFFSET = UINT32_MAX;
    static const FileSizeType INVALID_BYTE_SIZE = UINT32_MAX;
    static const uint32_t INVALID_AIO_TAG_ID = UINT32_MAX;
};
} // namespace pos
