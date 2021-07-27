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

#include "src/allocator/context_manager/file_io_manager.h"

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/metafs/metafs_file_intf.h"

namespace pos
{
AllocatorFileIoManager::AllocatorFileIoManager(MetaFileIntf** fileIntf, AllocatorAddressInfo* info, std::string arrayName)
: addrInfo(info),
  arrayName(arrayName),
  initialized(false)
{
    for (int file = 0; file < NUM_FILES; file++)
    {
        ctxFile[file] = fileIntf[file]; // for UT
        fileSize[file] = 0;
        numSections[file] = 0;
    }
}
AllocatorFileIoManager::AllocatorFileIoManager(AllocatorAddressInfo* info, std::string arrayName)
: addrInfo(info),
  arrayName(arrayName),
  initialized(false)
{
    for (int file = 0; file < NUM_FILES; file++)
    {
        ctxFile[file] = nullptr;
        fileSize[file] = 0;
        numSections[file] = 0;
    }
}

AllocatorFileIoManager::~AllocatorFileIoManager(void)
{
    Dispose();
}

void
AllocatorFileIoManager::Init(void)
{
    if (initialized == false)
    {
        for (int file = 0; file < NUM_FILES; file++)
        {
            fileSize[file] = 0;
            numSections[file] = 0;
            if (ctxFile[file] == nullptr)
            {
                ctxFile[file] = new FILESTORE(ctxFileName[file], arrayName);
            }
        }

        initialized = true;
    }
}

void
AllocatorFileIoManager::UpdateSectionInfo(int owner, int section, char* addr, int size, int offset)
{
    ctxSection[owner][section].Set(addr, size, offset);
    fileSize[owner] += size;
    numSections[owner]++;
}

void
AllocatorFileIoManager::Dispose(void)
{
    if (initialized == true)
    {
        for (int file = 0; file < NUM_FILES; file++)
        {
            if (ctxFile[file] != nullptr)
            {
                if (ctxFile[file]->IsOpened() == true)
                {
                    ctxFile[file]->Close();
                }
                delete ctxFile[file];
                ctxFile[file] = nullptr;
            }
        }

        initialized = false;
    }
}

int
AllocatorFileIoManager::LoadSync(int owner, char* buf)
{
    if (ctxFile[owner]->DoesFileExist() == false)
    {
        int ret = ctxFile[owner]->Create(fileSize[owner]);
        if (ret == 0)
        {
            ctxFile[owner]->Open();
            return 0;
        }
        else
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR), "Failed to create file:{}, size:{}", ctxFileName[owner], fileSize[owner]);
            return -1;
        }
    }
    else
    {
        ctxFile[owner]->Open();
        int ret = ctxFile[owner]->IssueIO(MetaFsIoOpcode::Read, 0, fileSize[owner], buf);
        if (ret == 0)
        {
            return 1;
        }
        else
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR), "Failed to Load file:{}, size:{}", ctxFileName[owner], fileSize[owner]);
            return -1;
        }
    }
}

int
AllocatorFileIoManager::StoreSync(int owner, char* buf)
{
    return ctxFile[owner]->IssueIO(MetaFsIoOpcode::Write, 0, fileSize[owner], buf);
}

int
AllocatorFileIoManager::StoreAsync(int owner, char* buf, MetaIoCbPtr callback)
{
    AllocatorIoCtx* request = new AllocatorIoCtx(MetaFsIoOpcode::Write,
        ctxFile[owner]->GetFd(), 0, fileSize[owner], buf, callback);
    int ret = ctxFile[owner]->AsyncIO(request);
    if (ret != 0)
    {
        delete request;
        POS_TRACE_ERROR(EID(FAILED_TO_ISSUE_ASYNC_METAIO), "Failed to issue AsyncMetaIo:{}, fname:{}", ret, ctxFileName[owner]);
    }
    return ret;
}

void
AllocatorFileIoManager::LoadSectionData(int owner, char* buf)
{
    for (int section = 0; section < numSections[owner]; section++)
    {
        memcpy(ctxSection[owner][section].addr, (buf + ctxSection[owner][section].offset), ctxSection[owner][section].size);
    }
}

void
AllocatorFileIoManager::CopySectionData(int owner, char* buf, int startSection, int endSection)
{
    for (int section = startSection; section < endSection; section++)
    {
        memcpy((buf + ctxSection[owner][section].offset), ctxSection[owner][section].addr, ctxSection[owner][section].size);
    }
}

int
AllocatorFileIoManager::GetFileSize(int owner)
{
    return fileSize[owner];
}

char*
AllocatorFileIoManager::GetSectionAddr(int owner, int section)
{
    return ctxSection[owner][section].addr;
}

int
AllocatorFileIoManager::GetSectionSize(int owner, int section)
{
    return ctxSection[owner][section].size;
}

int
AllocatorFileIoManager::GetSectionOffset(int owner, int section)
{
    return ctxSection[owner][section].offset;
}
//----------------------------------------------------------------------------//
} // namespace pos
