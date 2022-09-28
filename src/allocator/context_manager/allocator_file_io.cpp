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
#include "src/metafs/metafs_file_intf.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/config/metafs_config_manager.h"
#include "src/meta_file_intf/rocksdb_metafs_intf.h"

namespace pos
{
AllocatorFileIo::AllocatorFileIo(int owner, IAllocatorFileIoClient* client_, AllocatorAddressInfo* info, int arrayId_)
: AllocatorFileIo(owner, client_, info, nullptr)
{
    arrayId = arrayId_;
}

AllocatorFileIo::AllocatorFileIo(int owner, IAllocatorFileIoClient* client_, AllocatorAddressInfo* info, MetaFileIntf* file_)
: arrayId(UINT32_MAX),
  addrInfo(info),
  client(client_),
  owner(owner),
  file(file_),
  fileSize(0),
  numFilesReading(0),
  numFilesFlushing(0),
  initialized(false),
  rocksDbEnabled(MetaFsServiceSingleton::Instance()->GetConfigManager()->IsRocksdbEnabled())
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

    _UpdateSectionInfo();
    _CreateFile();

    initialized = true;
}

void
AllocatorFileIo::_UpdateSectionInfo(void)
{
    int currentOffset = 0;
    for (int sectionId = 0; sectionId < client->GetNumSections(); sectionId++)
    {
        int size = client->GetSectionSize(sectionId);

        ContextSection section(client->GetSectionAddr(sectionId), size, currentOffset);
        sections.push_back(section);

        currentOffset += size;
    }

    fileSize = currentOffset;
}

void
AllocatorFileIo::_CreateFile(void)
{
    if (file == nullptr)
    {
        if (rocksDbEnabled)
        {
            file = new RocksDBMetaFsIntf(client->GetFilename(), arrayId, MetaFileType::SpecialPurposeMap);
            POS_TRACE_INFO(EID(ALLOCATOR_FILE_IO_INITIALIZED),
                "RocksDBMetaFsIntf for allocator file io has been initialized , fileName : {} , arrayId : {} ", client->GetFilename(), arrayId);
        }
        else
        {
            file = new MetaFsFileIntf(client->GetFilename(), arrayId, MetaFileType::SpecialPurposeMap);
            POS_TRACE_INFO(EID(ALLOCATOR_FILE_IO_INITIALIZED),
                "MetaFsFileIntf for allocator file io has been initialized , fileName : {} , arrayId : {} ", client->GetFilename(), arrayId);
        }
    }
}

void
AllocatorFileIo::_LoadSectionData(char* buf)
{
    for (int sectionId = 0; sectionId < client->GetNumSections(); sectionId++)
    {
        if (sections[sectionId].addr != nullptr)
        {
            memcpy(sections[sectionId].addr, (buf + sections[sectionId].offset), sections[sectionId].size);
        }
    }
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

    sections.clear();

    initialized = false;
}

int
AllocatorFileIo::LoadContext(void)
{
    char* buf = new char[fileSize]();

    int ret = _Load(buf);
    if (ret != 1) // case for creating new file
    {
        delete[] buf;
    }
    return ret;
}

int
AllocatorFileIo::_Load(char* buf)
{
    if (file->DoesFileExist() == false)
    {
        int ret = file->Create(fileSize);
        if (ret == 0)
        {
            file->Open();
            return 0;
        }
        else
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR),
                "[AllocatorFileIo] Failed to create file:{}, size:{}",
                client->GetFilename(), fileSize);
            return -1;
        }
    }
    else
    {
        file->Open();

        numFilesReading++;

        MetaIoCbPtr callback = std::bind(&AllocatorFileIo::_LoadCompletedThenCB, this, std::placeholders::_1);
        AllocatorIoCtx* request = new AllocatorIoCtx(MetaFsIoOpcode::Read,
            file->GetFd(), 0, fileSize, buf, callback);
        int ret = file->AsyncIO(request);
        if (ret == 0)
        {
            return 1;
        }
        else
        {
            numFilesReading--;
            delete request;
            POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR),
                "[AllocatorFileIo] Failed to issue load:{}, fname:{}, size:{}",
                ret, client->GetFilename(), fileSize);
            return -1;
        }
    }
}

void
AllocatorFileIo::_LoadCompletedThenCB(AsyncMetaFileIoCtx* ctx)
{
    CtxHeader* header = reinterpret_cast<CtxHeader*>(ctx->buffer);
    assert(header->sig == client->GetSignature());

    _AfterLoad(ctx->buffer);
    POS_TRACE_INFO(EID(ALLOCATOR_META_ASYNCLOAD), "[AllocatorLoad] Async allocator file:{} load done!", owner);

    delete[] ctx->buffer;
    delete ctx;
}

void
AllocatorFileIo::_AfterLoad(char* buffer)
{
    int result = numFilesReading.fetch_sub(1) - 1;
    assert(result >= 0);

    _LoadSectionData(buffer);
    client->AfterLoad(buffer);
}

int
AllocatorFileIo::Flush(AllocatorCtxIoCompletion clientCallback, int dstSectionId, char* externalBuf)
{
    char* buf = new char[fileSize]();
    _PrepareBuffer(buf);

    if ((nullptr != externalBuf) && (INVALID_SECTION_ID != dstSectionId))
    {
        memcpy((buf + sections[dstSectionId].offset),
            externalBuf,
            sections[dstSectionId].size);
    }

    numFilesFlushing++;
    MetaIoCbPtr callback = std::bind(&AllocatorFileIo::_FlushCompletedThenCB, this, std::placeholders::_1);
    AllocatorIoCtx* request = new AllocatorIoCtx(MetaFsIoOpcode::Write,
        file->GetFd(), 0, fileSize, buf, callback, clientCallback);
    int ret = file->AsyncIO(request);
    if (ret != 0)
    {
        numFilesFlushing--;
        POS_TRACE_ERROR(EID(FAILED_TO_ISSUE_ASYNC_METAIO),
            "[AllocatorFileIo] Failed to issue store:{}, fname:{}", ret, client->GetFilename());

        delete request;
        delete[] buf;

        ret = -1;
    }
    return ret;
}

void
AllocatorFileIo::_FlushCompletedThenCB(AsyncMetaFileIoCtx* ctx)
{
    CtxHeader* header = reinterpret_cast<CtxHeader*>(ctx->buffer);
    assert(header->sig == client->GetSignature());

    _AfterFlush(ctx);
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE),
        "[AllocatorFlush] File flushed, sig:{}, version:{}", header->sig, header->ctxVersion);

    AllocatorIoCtx* ioContext = reinterpret_cast<AllocatorIoCtx*>(ctx);
    ioContext->clientCallback();

    delete[] ctx->buffer;
    delete ctx;
}

void
AllocatorFileIo::_AfterFlush(AsyncMetaFileIoCtx* ctx)
{
    int result = numFilesFlushing.fetch_sub(1) - 1;
    assert(result >= 0);

    client->FinalizeIo(ctx);
}

void
AllocatorFileIo::_PrepareBuffer(char* buf)
{
    client->BeforeFlush(buf);

    if (owner != REBUILD_CTX)
    {
        std::lock_guard<std::mutex> lock(client->GetCtxLock());
        _CopySectionData(buf, 0, client->GetNumSections());
    }
}

uint64_t
AllocatorFileIo::GetStoredVersion(void)
{
    return client->GetStoredVersion();
}

char*
AllocatorFileIo::GetSectionAddr(int section)
{
    return sections[section].addr;
}

int
AllocatorFileIo::GetSectionSize(int section)
{
    return sections[section].size;
}

int
AllocatorFileIo::GetDstSectionIdForExternalBufCopy(void)
{
    int sectionId = INVALID_SECTION_ID;

    switch (owner)
    {
        case SEGMENT_CTX:
            sectionId = SC_SEGMENT_INFO;
            break;

        default:
            sectionId = INVALID_SECTION_ID;
            break;
    }
    return sectionId;
}

void
AllocatorFileIo::_CopySectionData(char* buf, int startSection, int endSection)
{
    for (int section = startSection; section < endSection; section++)
    {
        if (sections[section].addr != nullptr)
        {
            memcpy((buf + sections[section].offset), sections[section].addr, sections[section].size);
        }
    }
}

int
AllocatorFileIo::GetNumFilesReading(void)
{
    return numFilesReading;
}

int
AllocatorFileIo::GetNumFilesFlushing(void)
{
    return numFilesFlushing;
}

} // namespace pos
