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

#include "file_descriptor_allocator.h"

#include <set>
#include <unordered_map>

#include "metafs_common_const.h"
#include "mf_inode.h"
#include "src/logger/logger.h"
#include "src/metafs/common/meta_file_util.h"

namespace pos
{
FileDescriptorAllocator::FileDescriptorAllocator(
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap,
    std::set<FileDescriptorType>* freeMap)
: fileKey2FdLookupMap(lookupMap),
  freeFdMap(freeMap)
{
    if (nullptr == lookupMap)
    {
        fileKey2FdLookupMap = new std::unordered_map<StringHashType, FileDescriptorType>();
    }

    if (nullptr == freeMap)
    {
        freeFdMap = new std::set<FileDescriptorType>();
    }

    assert(fileKey2FdLookupMap != nullptr);
    assert(freeFdMap != nullptr);

    Reset();
}

FileDescriptorAllocator::~FileDescriptorAllocator(void)
{
    if (nullptr != freeFdMap)
    {
        freeFdMap->clear();
        delete freeFdMap;
    }

    if (nullptr != fileKey2FdLookupMap)
    {
        fileKey2FdLookupMap->clear();
        delete fileKey2FdLookupMap;
    }
}
FileDescriptorType
FileDescriptorAllocator::Alloc(const StringHashType fileKey)
{
    FileDescriptorType fd = *(freeFdMap->begin());

    freeFdMap->erase(fd);
    fileKey2FdLookupMap->insert({fileKey, fd});

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "alloc new fd={} by fileKey={}", fd, fileKey);

    return fd;
}

FileDescriptorType
FileDescriptorAllocator::Alloc(const std::string& fileName)
{
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
    FileDescriptorType fd = Alloc(fileKey);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "alloc new fd={} by fileName={}", fd, fileName);

    return fd;
}

void
FileDescriptorAllocator::Free(const std::string& fileName, const FileDescriptorType fd)
{
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);

    Free(fileKey, fd);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "free fd={} by fileName={}", fd, fileName);
}

void
FileDescriptorAllocator::Free(const StringHashType fileKey, const FileDescriptorType fd)
{
    if (fd == fileKey2FdLookupMap->at(fileKey))
    {
        fileKey2FdLookupMap->erase(fileKey);
        freeFdMap->insert(fd);

        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "free fd={} by fileKey={}", fd, fileKey);
    }
    else
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_FILE_DESCRIPTOR_NOT_EXIST,
            "fd={}, fileKey={} doesn't exist", fd, fileKey);
    }
}

FileDescriptorType
FileDescriptorAllocator::FindFdByName(const std::string& fileName) const
{
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
    return FindFdByHashKey(fileKey);
}

FileDescriptorType
FileDescriptorAllocator::FindFdByHashKey(const StringHashType fileKey) const
{
    auto item = fileKey2FdLookupMap->find(fileKey);

    if (item == fileKey2FdLookupMap->end())
    {
        return MetaFsCommonConst::INVALID_FD;
    }

    return item->second;
}

bool
FileDescriptorAllocator::IsGivenFileCreated(const std::string& fileName) const
{
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
    return IsGivenFileCreated(fileKey);
}

bool
FileDescriptorAllocator::IsGivenFileCreated(const StringHashType fileKey) const
{
    FileDescriptorType fd = FindFdByHashKey(fileKey);
    return _IsFdValid(fd);
}

bool
FileDescriptorAllocator::_IsFdValid(const FileDescriptorType fd) const
{
    return (fd != MetaFsCommonConst::INVALID_FD && fd < MetaFsConfig::MAX_META_FILE_NUM_SUPPORT) ? true : false;
}

void
FileDescriptorAllocator::Reset(void)
{
    freeFdMap->clear();
    fileKey2FdLookupMap->clear();

    FileDescriptorType maxFile = MetaFsConfig::MAX_META_FILE_NUM_SUPPORT;
    for (FileDescriptorType fd = 0; fd < maxFile; fd++)
    {
        freeFdMap->insert(fd);
    }
}

void
FileDescriptorAllocator::UpdateFreeMap(const FileDescriptorType fd)
{
    assert(freeFdMap->find(fd) != freeFdMap->end());
    freeFdMap->erase(fd);
}

void
FileDescriptorAllocator::UpdateLookupMap(const StringHashType fileKey,
    const FileDescriptorType fd)
{
    fileKey2FdLookupMap->insert({fileKey, fd});
}

uint32_t
FileDescriptorAllocator::GetMaxFileCount(void) const
{
    return MetaFsConfig::MAX_META_FILE_NUM_SUPPORT;
}

// only for test
std::unordered_map<StringHashType, FileDescriptorType>*
FileDescriptorAllocator::GetLookupMap(void)
{
    return fileKey2FdLookupMap;
}

// only for test
std::set<FileDescriptorType>*
FileDescriptorAllocator::GetFreeMap(void)
{
    return freeFdMap;
}
} // namespace pos
