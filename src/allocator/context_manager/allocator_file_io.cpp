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

#include "src/allocator/context_manager/allocator_file_io.h"

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/meta_file_intf/meta_file_intf.h"
#include "src/metafs/include/meta_file_property.h"
#include "src/metafs/metafs_file_intf.h"
#include "src/metafs/config/metafs_config_manager.h"
#include "src/meta_file_intf/rocksdb_metafs_intf.h"

namespace pos
{
AllocatorFileIo::AllocatorFileIo(FileOwner owner, IAllocatorFileIoClient* client_, AllocatorAddressInfo* info)
: AllocatorFileIo(owner, client_, info, nullptr)
{
}

AllocatorFileIo::AllocatorFileIo(FileOwner owner, IAllocatorFileIoClient* client_, AllocatorAddressInfo* info, MetaFileIntf* file_)
: addrInfo(info),
  client(client_),
  owner(owner),
  file(file_),
  numOutstandingReads(0),
  numOutstandingFlushes(0),
  initialized(false)
{
}

AllocatorFileIo::~AllocatorFileIo(void)
{
    Dispose();
}

void
AllocatorFileIo::Init(void)
{
    if (initialized == true)
    {
        return;
    }

    if (file == nullptr)
    {
        file = MetaFileIntf::CreateFileIntf(ToFilename(owner), addrInfo->GetArrayId(), MetaFileType::SpecialPurposeMap);
    }

    initialized = true;
}

void
AllocatorFileIo::Dispose(void)
{
    if (initialized == false)
    {
        return;
    }

    if (file != nullptr)
    {
        if (file->IsOpened() == true)
        {
            file->Close();
        }
        delete file;
        file = nullptr;
    }
    initialized = false;
}

int
AllocatorFileIo::LoadContext(void)
{
    int ret = EID(SUCCESS);
    if (file->DoesFileExist() == false)
    {
        ret = _CreateAndOpenFile();
        if (ret == EID(SUCCESS))
        {
            ret = EID(SUCCEED_TO_OPEN_WITH_CREATION);
        }
    }
    else
    {
        ret = _OpenAndLoadFile();
        if (ret == EID(SUCCESS))
        {
            ret = EID(SUCCEED_TO_OPEN_WITHOUT_CREATION);
        }
    }
    return ret;
}

int
AllocatorFileIo::_CreateAndOpenFile(void)
{
    uint64_t fileSize = client->GetTotalDataSize();

    int ret = file->Create(fileSize);
    if (ret == EID(SUCCESS))
    {
        ret = file->Open();
        if (EID(SUCCESS) != ret)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR),
                "Failed to open file:{}, size:{}, error:{}",
                file->GetFileName(), fileSize, ret);
        }
    }
    else
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR),
            "Failed to create file:{}, size:{}, error:{}",
            file->GetFileName(), fileSize, ret);
    }

    return ret;
}

int
AllocatorFileIo::_OpenAndLoadFile(void)
{
    int ret = file->Open();
    if (EID(SUCCESS) != ret)
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR),
            "Failed to open file:{}, error:{}",
            file->GetFileName(), ret);
        return ret;
    }
    return _Load();
}

int
AllocatorFileIo::_Load(void)
{
    numOutstandingReads++;

    uint64_t size = client->GetTotalDataSize();

    char* buf = new char[size]();
    FnCompleteMetaFileIo callback = std::bind(&AllocatorFileIo::_LoadCompletedThenCB, this, std::placeholders::_1);
    AsyncMetaFileIoCtx* request = new AsyncMetaFileIoCtx();
    request->SetIoInfo(MetaFsIoOpcode::Read, 0, size, buf);
    request->SetFileInfo(file->GetFd(), file->GetIoDoneCheckFunc());
    request->SetCallback(callback);

    int ret = file->AsyncIO(request);
    if (ret != EID(SUCCESS))
    {
        numOutstandingReads--;
        POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR),
            "Failed to issue load, fname:{}, offset:0, size:{}, error:{}",
            file->GetFileName(), size, ret);
        POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR), request->ToString());
        delete[] buf;
        delete request;
    }

    return ret;
}

void
AllocatorFileIo::_LoadCompletedThenCB(AsyncMetaFileIoCtx* ctx)
{
    int result = numOutstandingReads.fetch_sub(1) - 1;
    assert(result >= 0);

    char* buffer = ctx->GetBuffer();
    client->AfterLoad(buffer);

    POS_TRACE_INFO(EID(ALLOCATOR_META_ASYNCLOAD),
        "[AllocatorLoad] Async allocator file:{} load done!", ToFilename(owner));

    delete[] buffer;
    delete ctx;
}

int
AllocatorFileIo::Flush(FnAllocatorCtxIoCompletion clientCallback, ContextSectionBuffer externalBuf)
{
    uint64_t size = client->GetTotalDataSize();

    char* buf = new char[size]();
    client->BeforeFlush(buf, externalBuf);

    numOutstandingFlushes++;
    FnCompleteMetaFileIo callback = std::bind(&AllocatorFileIo::_FlushCompletedThenCB, this, std::placeholders::_1);
    AsyncMetaFileIoCtx* request = new AllocatorIoCtx(clientCallback);
    request->SetIoInfo(MetaFsIoOpcode::Write, 0, size, buf);
    request->SetFileInfo(file->GetFd(), file->GetIoDoneCheckFunc());
    request->SetCallback(callback);

    int ret = file->AsyncIO(request);
    if (ret != EID(SUCCESS))
    {
        numOutstandingFlushes--;
        POS_TRACE_ERROR(EID(FAILED_TO_ISSUE_ASYNC_METAIO),
            "Failed to issue store:{}, fname:{}", ret, file->GetFileName());
        POS_TRACE_ERROR(EID(FAILED_TO_ISSUE_ASYNC_METAIO), request->ToString());

        delete request;
        delete[] buf;

        // TODO call client callback with error
    }
    return ret;
}

void
AllocatorFileIo::_FlushCompletedThenCB(AsyncMetaFileIoCtx* ctx)
{
    int result = numOutstandingFlushes.fetch_sub(1) - 1;
    assert(result >= 0);

    char* buffer = ctx->GetBuffer();
    client->AfterFlush(buffer);

    AllocatorIoCtx* ioContext = reinterpret_cast<AllocatorIoCtx*>(ctx);
    (ioContext->GetAllocatorClientCallback())();

    delete[] buffer;
    delete ctx;
}

uint64_t
AllocatorFileIo::GetStoredVersion(void)
{
    return client->GetStoredVersion();
}

int
AllocatorFileIo::GetSectionSize(int section)
{
    return client->GetSectionInfo(section).size;
}

int
AllocatorFileIo::GetNumOutstandingRead(void)
{
    return numOutstandingReads;
}

int
AllocatorFileIo::GetNumOutstandingFlush(void)
{
    return numOutstandingFlushes;
}

} // namespace pos
