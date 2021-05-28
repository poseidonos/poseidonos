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
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/metafs/metafs_file_intf.h"

namespace pos
{
AllocatorFileIoManager::AllocatorFileIoManager(MetaFileIntf** fileIntf, AllocatorAddressInfo* info)
: addrInfo(info)
{
    // This constructor is only for UT !!
    arrayName = "";
    for (int owner = 0; owner < NUM_FILES; owner++)
    {
        fileInfo[owner].fileName = "";
        fileInfo[owner].size = 0;
        fileInfo[owner].numSections = 0;
        if (fileIntf != nullptr)
        {
            fileInfo[owner].file = fileIntf[owner];
        }
    }
}

AllocatorFileIoManager::AllocatorFileIoManager(std::string* fileNames, AllocatorAddressInfo* info, std::string arrayName)
: addrInfo(info),
  arrayName(arrayName)
{
    for (int owner = 0; owner < NUM_FILES; owner++)
    {
        fileInfo[owner].fileName = fileNames[owner];
        fileInfo[owner].size = 0;
        fileInfo[owner].numSections = 0;
        fileInfo[owner].file = nullptr;
    }
}

AllocatorFileIoManager::~AllocatorFileIoManager(void)
{
}

void
AllocatorFileIoManager::Init(void)
{
    for (int owner = 0; owner < NUM_FILES; owner++)
    {
        fileInfo[owner].size = 0;
        fileInfo[owner].numSections = 0;
        if (fileInfo[owner].file == nullptr)
        {
            fileInfo[owner].file = new FILESTORE(fileInfo[owner].fileName, arrayName);
        }
    }
}

void
AllocatorFileIoManager::UpdateSectionInfo(int owner, int section, char* addr, int size, int offset)
{
    fileInfo[owner].section[section].Set(addr, size, offset);
    fileInfo[owner].size += size;
    fileInfo[owner].numSections++;
}

void
AllocatorFileIoManager::Close(void)
{
    for (int owner = 0; owner < NUM_FILES; owner++)
    {
        if (fileInfo[owner].file != nullptr)
        {
            if (fileInfo[owner].file->IsOpened() == true)
            {
                fileInfo[owner].file->Close();
            }
            delete fileInfo[owner].file;
            fileInfo[owner].file = nullptr;
        }
    }
}

int
AllocatorFileIoManager::LoadSync(int owner, char* buf)
{
    if (fileInfo[owner].file->DoesFileExist() == false)
    {
        int ret = fileInfo[owner].file->Create(fileInfo[owner].size);
        if (ret == 0)
        {
            fileInfo[owner].file->Open();
            return 0;
        }
        else
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR), "Failed to create file:{}, size:{}", fileInfo[owner].fileName, fileInfo[owner].size);
            return -1;
        }
    }
    else
    {
        fileInfo[owner].file->Open();
        int ret = fileInfo[owner].file->IssueIO(MetaFsIoOpcode::Read, 0, fileInfo[owner].size, buf);
        if (ret == 0)
        {
            return 1;
        }
        else
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR), "Failed to load file:{}, size:{}", fileInfo[owner].fileName, fileInfo[owner].size);
            return -1;
        }
    }
}

int
AllocatorFileIoManager::StoreSync(int owner, char* buf)
{
    return fileInfo[owner].file->IssueIO(MetaFsIoOpcode::Write, 0, fileInfo[owner].size, buf);
}

int
AllocatorFileIoManager::StoreAsync(int owner, char* buf, MetaIoCbPtr callback)
{
    AllocatorIoCtx* request = new AllocatorIoCtx(MetaFsIoOpcode::Write,
        fileInfo[owner].file->GetFd(), 0, fileInfo[owner].size, buf, callback);
    int ret = fileInfo[owner].file->AsyncIO(request);
    if (ret != 0)
    {
        delete request;
        POS_TRACE_ERROR(EID(FAILED_TO_ISSUE_ASYNC_METAIO), "Failed to issue AsyncMetaIo:{}, fname:{}", ret, fileInfo[owner].fileName);
    }
    return ret;
}

void
AllocatorFileIoManager::LoadSectionData(int owner, char* buf)
{
    for (int section = 0; section < fileInfo[owner].numSections; section++)
    {
        if (fileInfo[owner].section[section].addr != nullptr)
        {
            memcpy(fileInfo[owner].section[section].addr, (buf + fileInfo[owner].section[section].offset), fileInfo[owner].section[section].size);
        }
    }
}

void
AllocatorFileIoManager::CopySectionData(int owner, char* buf, int startSection, int endSection)
{
    for (int section = startSection; section < endSection; section++)
    {
        if (fileInfo[owner].section[section].addr != nullptr)
        {
            memcpy((buf + fileInfo[owner].section[section].offset), fileInfo[owner].section[section].addr, fileInfo[owner].section[section].size);
        }
    }
}

int
AllocatorFileIoManager::GetFileSize(int owner)
{
    return fileInfo[owner].size;
}

char*
AllocatorFileIoManager::GetSectionAddr(int owner, int section)
{
    return fileInfo[owner].section[section].addr;
}

int
AllocatorFileIoManager::GetSectionSize(int owner, int section)
{
    return fileInfo[owner].section[section].size;
}

int
AllocatorFileIoManager::GetSectionOffset(int owner, int section)
{
    return fileInfo[owner].section[section].offset;
}
//----------------------------------------------------------------------------//
} // namespace pos
