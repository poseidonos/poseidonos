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

#include <set>
#include <string>
#include <unordered_map>

#include "metafs_common.h"

namespace pos
{
using StringHashMap = std::unordered_map<StringHashType, FileDescriptorType>;

// File Descriptor Allocator
class FileDescriptorAllocator
{
public:
    FileDescriptorAllocator(
        std::unordered_map<StringHashType, FileDescriptorType>* lookupMap = nullptr,
        std::set<FileDescriptorType>* freeMap = nullptr);
    virtual ~FileDescriptorAllocator(void);

    virtual FileDescriptorType Alloc(std::string& fileName);
    virtual FileDescriptorType Alloc(StringHashType fileKey);

    virtual void Free(std::string& fileName, FileDescriptorType fd);
    virtual void Free(StringHashType fileKey, FileDescriptorType fd);

    virtual FileDescriptorType FindFdByName(std::string& fileName);
    virtual FileDescriptorType FindFdByHashKey(StringHashType fileKey);

    virtual bool IsGivenFileCreated(std::string& fileName);
    virtual bool IsGivenFileCreated(StringHashType fileKey);

    virtual void Reset(void);
    virtual void UpdateFreeMap(FileDescriptorType fd);
    virtual void UpdateLookupMap(StringHashType fileKey, FileDescriptorType fd);
    virtual uint32_t GetMaxFileCount(void);

    // only for test
    virtual std::unordered_map<StringHashType, FileDescriptorType>* GetLookupMap(void);
    // only for test
    virtual std::set<FileDescriptorType>* GetFreeMap(void);

private:
    bool _IsFdValid(FileDescriptorType fd);

    std::unordered_map<StringHashType, FileDescriptorType>* fileKey2FdLookupMap;
    std::set<FileDescriptorType>* freeFdMap;
};
} // namespace pos
