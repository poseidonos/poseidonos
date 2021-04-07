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

#include "meta_fd_mgr.h"

#include "mf_inode.h"
#include "mfs_common_const.h"
#include "src/logger/logger.h"

MetaFDMgrClass::MetaFDMgrClass(void)
{
}
MetaFDMgrClass::~MetaFDMgrClass(void)
{
    freeFDMap.clear();
    fileKey2FDLookupMap.clear();
}

FileFDType
MetaFDMgrClass::AllocNewFD(void)
{
    FileFDType fd = freeFDMap.begin()->second;
    freeFDMap.erase(fd);

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "New FD allocated = {}", fd);

    return fd;
}

void
MetaFDMgrClass::FreeFD(FileFDType fd)
{
    freeFDMap.insert(std::make_pair(fd, fd));
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Free FD = {}", fd);
}

void
MetaFDMgrClass::InsertFileDescLookupHash(StringHashType fileKey, FileFDType fd)
{
    fileKey2FDLookupMap.insert(std::make_pair(fileKey, fd));
}

void
MetaFDMgrClass::EraseFileDescLookupHash(StringHashType fileKey)
{
    fileKey2FDLookupMap.erase(fileKey);
}

FileFDType
MetaFDMgrClass::FindFDByName(StringHashType fileKey)
{
    auto item = fileKey2FDLookupMap.find(fileKey);

    if (item == fileKey2FDLookupMap.end())
    {
        return MetaFsCommonConst::INVALID_FD;
    }
    FileFDType fd = item->second;
    return fd;
}

bool
MetaFDMgrClass::IsGivenFileCreated(StringHashType fileKey)
{
    FileFDType fd = FindFDByName(fileKey);

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
MetaFDMgrClass::_IsFDValid(FileFDType fd)
{
    return (fd != MetaFsCommonConst::INVALID_FD && fd < MetaFsConfig::MAX_META_FILE_NUM_SUPPORT) ? true : false;
}

void
MetaFDMgrClass::AddAllFDsInFreeFDMap(void)
{
    for (FileFDType fd = 0; fd < MetaFsConfig::MAX_META_FILE_NUM_SUPPORT; fd++)
    {
        freeFDMap.insert(std::make_pair(fd, fd));
    }
}

std::map<FileFDType, FileFDType>&
MetaFDMgrClass::GetFreeFDMap(void)
{
    return freeFDMap;
}

std::unordered_map<StringHashType, FileFDType>&
MetaFDMgrClass::GetFDLookupMap(void)
{
    return fileKey2FDLookupMap;
}

void
MetaFDMgrClass::Reset(void)
{
    freeFDMap.clear();
    fileKey2FDLookupMap.clear();
}
