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

#include "file_descriptor_manager.h"

#include "mf_inode.h"
#include "metafs_common_const.h"
#include "src/logger/logger.h"

namespace pos
{
FileDescriptorManager::FileDescriptorManager(
        std::unordered_map<StringHashType, FileDescriptorType>* lookupMap,
        std::map<FileDescriptorType, FileDescriptorType>* freeMap)
: fileKey2FDLookupMap(lookupMap),
  freeFDMap(freeMap)
{
    if (nullptr == lookupMap)
    {
        fileKey2FDLookupMap = new std::unordered_map<StringHashType, FileDescriptorType>();
    }

    if (nullptr == freeMap)
    {
        freeFDMap = new std::map<FileDescriptorType, FileDescriptorType>();
    }
}

FileDescriptorManager::~FileDescriptorManager(void)
{
    if (nullptr != freeFDMap)
    {
        freeFDMap->clear();
        delete freeFDMap;
    }

    if (nullptr != fileKey2FDLookupMap)
    {
        fileKey2FDLookupMap->clear();
        delete fileKey2FDLookupMap;
    }
}

FileDescriptorType
FileDescriptorManager::Alloc(void)
{
    FileDescriptorType fd = freeFDMap->begin()->second;
    freeFDMap->erase(fd);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "New FD allocated = {}", fd);

    return fd;
}

void
FileDescriptorManager::Free(FileDescriptorType fd)
{
    freeFDMap->insert(std::make_pair(fd, fd));
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Free FD = {}", fd);
}

void
FileDescriptorManager::InsertFileDescLookupHash(StringHashType fileKey, FileDescriptorType fd)
{
    fileKey2FDLookupMap->insert(std::make_pair(fileKey, fd));
}

void
FileDescriptorManager::EraseFileDescLookupHash(StringHashType fileKey)
{
    fileKey2FDLookupMap->erase(fileKey);
}

FileDescriptorType
FileDescriptorManager::FindFDByName(StringHashType fileKey)
{
    auto item = fileKey2FDLookupMap->find(fileKey);

    if (item == fileKey2FDLookupMap->end())
    {
        return MetaFsCommonConst::INVALID_FD;
    }
    FileDescriptorType fd = item->second;
    return fd;
}

bool
FileDescriptorManager::IsGivenFileCreated(StringHashType fileKey)
{
    FileDescriptorType fd = FindFDByName(fileKey);

    if (_IsFDValid(fd))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool
FileDescriptorManager::_IsFDValid(FileDescriptorType fd)
{
    return (fd != MetaFsCommonConst::INVALID_FD && fd < MetaFsConfig::MAX_META_FILE_NUM_SUPPORT) ? true : false;
}

void
FileDescriptorManager::AddAllFDsInFreeFDMap(void)
{
    for (FileDescriptorType fd = 0; fd < MetaFsConfig::MAX_META_FILE_NUM_SUPPORT; fd++)
    {
        freeFDMap->insert(std::make_pair(fd, fd));
    }
}

std::map<FileDescriptorType, FileDescriptorType>&
FileDescriptorManager::GetFreeFDMap(void)
{
    return *freeFDMap;
}

std::unordered_map<StringHashType, FileDescriptorType>&
FileDescriptorManager::GetFDLookupMap(void)
{
    return *fileKey2FDLookupMap;
}

void
FileDescriptorManager::Reset(void)
{
    freeFDMap->clear();
    fileKey2FDLookupMap->clear();
}
} // namespace pos
