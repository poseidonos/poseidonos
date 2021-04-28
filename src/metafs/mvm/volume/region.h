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
#include "mdpage.h"
#include "mf_inode.h"
#include "src/metafs/storage/mss.h"
#include "region_content.h"

namespace pos
{
class Region
{
public:
    Region(std::string arrayName, MetaStorageType type, MetaFileInode* inode, uint64_t baseMetaLpn, uint64_t count, bool inUse, MetaStorageSubsystem* metaStorage)
    {
        this->arrayName = arrayName;

        regionType = type;
        mssIntf = metaStorage;

        content.SetInode(inode);
        content.SetBaseMetaLpn(baseMetaLpn);
        content.SetSize(count);
        content.SetInUse(inUse);

        if (inode != nullptr)
        {
            content.SetIndexInInodeTable(inode->data.basic.field.indexInInodeTable);
        }
    }

    bool
    operator<(const Region& a)
    {
        return content.GetBaseMetaLpn() < a.content.GetBaseMetaLpn();
    }

    Region*
    operator=(const Region& a)
    {
        regionType = a.regionType;
        content.SetInode(a.content.GetInode());
        content.SetBaseMetaLpn(a.content.GetBaseMetaLpn());
        content.SetSize(a.content.GetSize());
        content.SetInUse(a.content.GetInUse());

        return this;
    }

    RegionContent*
    GetContent(void)
    {
        return &content;
    }

    MetaFileInode*
    GetInode(void) const
    {
        return content.GetInode();
    }
    void
    SetInode(MetaFileInode* inode)
    {
        content.SetInode(inode);
    }

    uint64_t
    GetBaseMetaLpn(void) const
    {
        return content.GetBaseMetaLpn();
    }
    void
    SetBaseMetaLpn(uint64_t baseLpn)
    {
        content.SetBaseMetaLpn(baseLpn);
    }

    uint64_t
    GetLastMetaLpn(void) const
    {
        return content.GetLastMetaLpn();
    }

    uint64_t
    GetSize(void) const
    {
        return content.GetSize();
    }
    void
    SetSize(uint64_t cnt)
    {
        content.SetSize(cnt);
    }

    bool
    GetInUse(void) const
    {
        return content.GetInUse();
    }
    void
    SetInUse(bool inUse)
    {
        content.SetInUse(inUse);
    }

    void
    SetInvalid(void)
    {
        content.SetInUse(false);
        content.SetInode(nullptr);
        content.SetIndexInInodeTable(0);
    }

    bool Move(Region* target);
    POS_EVENT_ID Erase(MetaLpnType startLpn, MetaLpnType numTrimLpns);

protected:
    std::string arrayName;
    MetaStorageType regionType;
    RegionContent content;

private:
    MetaStorageSubsystem* mssIntf;
};
} // namespace pos
