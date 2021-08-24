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

#pragma once

#include <string>

#include "src/allocator/include/allocator_const.h"
#include "src/meta_file_intf/async_context.h"

namespace pos
{
class AllocatorAddressInfo;
class MetaFileIntf;

class AllocatorFileIoManager
{
    class ContextSection
    {
    public:
        ContextSection(void) : addr(nullptr), size(0), offset(0) {}
        void Set(char* addrIn, int sizeIn, int offsetIn)
        {
            addr = addrIn, size = sizeIn, offset = offsetIn; 
        }

        char* addr;
        int size;
        int offset;
    };
    struct FileInfo
    {
        std::string fileName;
        MetaFileIntf* file;
        ContextSection section[NUM_ALLOCATOR_CTX_SECTION];
        int size;
        int numSections;
    };

public:
    AllocatorFileIoManager(void) = default;
    AllocatorFileIoManager(MetaFileIntf** file, AllocatorAddressInfo* info); // only for UT
    AllocatorFileIoManager(std::string* fileNames, AllocatorAddressInfo* info, std::string arrayName);
    virtual ~AllocatorFileIoManager(void);
    virtual void Init(void);
    virtual void Dispose(void);

    virtual void UpdateSectionInfo(int owner, int section, char* addr, int size, int offset);

    virtual int Load(int owner, char* buf, MetaIoCbPtr callback);
    virtual int Store(int owner, char* buf, MetaIoCbPtr callback);

    virtual void LoadSectionData(int owner, char* buf);
    virtual void CopySectionData(int owner, char* buf, int startSection, int endSection);

    virtual int GetFileSize(int owner);
    virtual char* GetSectionAddr(int owner, int section);
    virtual int GetSectionSize(int owner, int section);
    virtual int GetSectionOffset(int owner, int section);

private:
    int _Flush(char* data, EventSmartPtr callback);

    FileInfo fileInfo[NUM_FILES];
    AllocatorAddressInfo* addrInfo;
    std::string arrayName;

    bool initialized;
};

} // namespace pos
