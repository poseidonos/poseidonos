/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include "src/metafs/mvm/volume/fd_inode_map.h"

#include <string>
#include <unordered_map>

#include "src/metafs/common/metafs_type.h"

namespace pos
{
FdInodeMap::FdInodeMap(void)
{
}

FdInodeMap::~FdInodeMap(void)
{
}

void
FdInodeMap::Add(const FileDescriptorType fd, MetaFileInode* inode)
{
    fd2InodeMap_.insert({fd, inode});
}

size_t
FdInodeMap::Remove(const FileDescriptorType fd)
{
    return fd2InodeMap_.erase(fd);
}

MetaFileInode*
FdInodeMap::GetInode(const FileDescriptorType fd) const
{
    auto item = fd2InodeMap_.find(fd);
    if (item == fd2InodeMap_.end())
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "File descriptor {} is not existed.", fd);
        assert(false);
        return nullptr;
    }
    return item->second;
}

std::string
FdInodeMap::GetFileName(const FileDescriptorType fd) const
{
    auto item = fd2InodeMap_.find(fd);
    if (item == fd2InodeMap_.end())
        return "";
    return item->second->data.basic.field.fileName.ToString();
}

void
FdInodeMap::Reset(void)
{
    fd2InodeMap_.clear();
}

std::unordered_map<FileDescriptorType, MetaFileInode*>&
FdInodeMap::GetInodeMap(void)
{
    return fd2InodeMap_;
}
} // namespace pos
