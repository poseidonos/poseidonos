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

#include "inode_table.h"

namespace pos
{
InodeTable::InodeTable(MetaVolumeType volumeType, MetaLpnType startLpn)
: OnVolumeMetaRegion<MetaRegionType, InodeTableContent>(volumeType, MetaRegionType::FileInodeTable, startLpn)
{
}

InodeTable::~InodeTable(void)
{
    delete content;
}

void
InodeTable::Create(uint32_t maxInodeEntryNum)
{
    MetaFileInodeArray& entries = GetContent()->entries;
    for (auto& inode : entries)
    {
        inode.CleanupEntry();
    }
}

FileDescriptorType
InodeTable::GetFileDescriptor(uint32_t inodeIdx)
{
    MetaFileInode& inode = GetContent()->entries[inodeIdx];
    return inode.data.basic.field.fd;
}

MetaFileInode&
InodeTable::GetInode(uint32_t inodeIdx)
{
    return GetContent()->entries[inodeIdx];
}

MetaFileInodeArray&
InodeTable::GetInodeArray(void)
{
    return GetContent()->entries;
}
} // namespace pos
