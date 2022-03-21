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

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/file_descriptor_allocator.h"

namespace pos
{
class MockFileDescriptorAllocator : public FileDescriptorAllocator
{
public:
    using FileDescriptorAllocator::FileDescriptorAllocator;

    MOCK_METHOD(FileDescriptorType, Alloc, (const std::string& fileName));
    MOCK_METHOD(FileDescriptorType, Alloc, (const StringHashType fileKey));

    MOCK_METHOD(void, Free, (const std::string& fileName, const FileDescriptorType fd));
    MOCK_METHOD(void, Free, (const StringHashType fileKey, const FileDescriptorType fd));

    MOCK_METHOD(FileDescriptorType, FindFdByName, (const std::string& fileName), (const));
    MOCK_METHOD(FileDescriptorType, FindFdByHashKey, (const StringHashType fileKey), (const));

    MOCK_METHOD(bool, IsGivenFileCreated, (const std::string& fileName), (const));
    MOCK_METHOD(bool, IsGivenFileCreated, (const StringHashType fileKey), (const));

    MOCK_METHOD(void, Reset, ());
    MOCK_METHOD(void, UpdateFreeMap, (const FileDescriptorType fd));
    MOCK_METHOD(void, UpdateLookupMap, (const StringHashType fileKey, const FileDescriptorType fd));
    MOCK_METHOD(uint32_t, GetMaxFileCount, (), (const));

    // only for test
    MOCK_METHOD(StringHashMap*, GetLookupMap, ());
    // only for test
    MOCK_METHOD(std::set<FileDescriptorType>*, GetFreeMap, ());
};

} // namespace pos
