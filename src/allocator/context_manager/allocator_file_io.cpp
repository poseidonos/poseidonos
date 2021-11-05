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
#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/meta_file_intf/meta_file_intf.h"
#include "src/metafs/metafs_file_intf.h"

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

    _UpdateSectionInfo();
    _CreateFile();

    initialized = true;
}

void
AllocatorFileIo::_UpdateSectionInfo(void)
{
    int currentOffset = 0;
    for (int sectionId = 0; sectionId < numSections[owner]; sectionId++)
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
        file = new MetaFsFileIntf(filenames[owner], arrayId);
    }
}

void
AllocatorFileIo::_LoadSectionData(char* buf)
{
    for (int sectionId = 0; sectionId < numSections[owner]; sectionId++)
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
AllocatorFileIo::LoadContext(MetaIoCbPtr callback)
{
    char* buf = new char[fileSize]();

    int ret = _Load(buf, callback);
    if (ret != 1) // case for creating new file
    {
        delete[] buf;
    }

    return ret;
}

int
AllocatorFileIo::_Load(char* buf, MetaIoCbPtr callback)
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
                filenames[owner], fileSize);
            return -1;
        }
    }
    else
    {
        file->Open();
        AllocatorIoCtx* request = new AllocatorIoCtx(MetaFsIoOpcode::Read,
            file->GetFd(), 0, fileSize, buf, callback);
        int ret = file->AsyncIO(request);
        if (ret == 0)
        {
            return 1;
        }
        else
        {
            delete request;
            POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR),
                "[AllocatorFileIo] Failed to issue load:{}, fname:{}, size:{}",
                ret, filenames[owner], fileSize);
            return -1;
        }
    }
}

void
AllocatorFileIo::AfterLoad(char* buffer)
{
    _LoadSectionData(buffer);
    client->AfterLoad(buffer);
}

int
AllocatorFileIo::Flush(MetaIoCbPtr callback)
{
    char* buf = new char[fileSize]();
    _PrepareBuffer(buf);

    AllocatorIoCtx* request = new AllocatorIoCtx(MetaFsIoOpcode::Write,
        file->GetFd(), 0, fileSize, buf, callback);
    int ret = file->AsyncIO(request);
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(FAILED_TO_ISSUE_ASYNC_METAIO),
            "[AllocatorFileIo] Failed to issue store:{}, fname:{}", ret, filenames[owner]);

        delete request;
        delete[] buf;

        ret = -1;
    }
    return ret;
}

void
AllocatorFileIo::AfterFlush(AsyncMetaFileIoCtx* ctx)
{
    client->FinalizeIo(ctx);
}

void
AllocatorFileIo::_PrepareBuffer(char* buf)
{
    client->BeforeFlush(buf);

    if (owner != REBUILD_CTX)
    {
        std::lock_guard<std::mutex> lock(client->GetCtxLock());
        _CopySectionData(buf, 0, numSections[owner]);
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
} // namespace pos
