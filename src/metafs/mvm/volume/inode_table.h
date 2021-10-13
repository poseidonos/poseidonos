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

#include "mf_inode.h"
#include "on_volume_meta_region.h"
#include "meta_region_type.h"

namespace pos
{
class InodeTableContent : public MetaRegionContent
{
public:
    static const uint32_t MAX_META_FILE_INODE_NUM = MetaFsConfig::MAX_META_FILE_NUM_SUPPORT;
    std::array<MetaFileInode, InodeTableContent::MAX_META_FILE_INODE_NUM> entries;
};
using MetaFileInodeArray = std::array<MetaFileInode, InodeTableContent::MAX_META_FILE_INODE_NUM>;

class InodeTable : public OnVolumeMetaRegion<MetaRegionType, InodeTableContent>
{
public:
    explicit InodeTable(MetaVolumeType volumeType, MetaLpnType startLpn);
    virtual ~InodeTable(void);

    virtual void Create(uint32_t maxInodeEntryNum);
    virtual FileDescriptorType GetFileDescriptor(uint32_t inodeIdx);
    virtual MetaFileInode& GetInode(uint32_t inodeIdx);
    virtual MetaFileInodeArray& GetInodeArray(void);
};
} // namespace pos
