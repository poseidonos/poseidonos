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

#include "inode_table_header.h"

namespace pos
{
InodeTableHeader::InodeTableHeader(MetaVolumeType volumeType, MetaLpnType baseLpn)
: OnVolumeMetaRegion<MetaRegionType, InodeTableHeaderContent>(volumeType, MetaRegionType::FileInodeHdr, baseLpn),
  freeInodeEntryIdxQ(new std::queue<uint32_t>)
{
}

InodeTableHeader::~InodeTableHeader(void)
{
    delete freeInodeEntryIdxQ;
    delete content;
}

void
InodeTableHeader::Create(uint32_t totalFileNum)
{
    ResetContent();

    InodeTableHeaderContent* content = GetContent();
    content->totalInodeNum = totalFileNum;

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Total Inode entry number available={}", totalFileNum);
}

void
InodeTableHeader::SetInodeInUse(uint32_t idx)
{
    InodeTableHeaderContent* content = GetContent();
    assert(false == content->inodeInUseBitmap.bits.test(idx));
    content->inodeInUseBitmap.bits.set(idx);
    content->inodeInUseBitmap.allocatedInodeCnt++;
    content->totalFileCreated++;
}

void
InodeTableHeader::ClearInodeInUse(uint32_t idx)
{
    InodeTableHeaderContent* content = GetContent();
    assert(true == content->inodeInUseBitmap.bits.test(idx));
    content->inodeInUseBitmap.bits.reset(idx);
    content->inodeInUseBitmap.allocatedInodeCnt--;
    content->totalFileCreated--;
    freeInodeEntryIdxQ->push(idx);
}

bool
InodeTableHeader::IsFileInodeInUse(uint32_t idx)
{
    InodeTableHeaderContent* content = GetContent();
    return content->inodeInUseBitmap.bits.test(idx);
}

uint32_t
InodeTableHeader::GetTotalAllocatedInodeCnt(void)
{
    InodeTableHeaderContent* content = GetContent();
    uint32_t allocatedInodeCnt = content->inodeInUseBitmap.allocatedInodeCnt;
    return allocatedInodeCnt;
}

void
InodeTableHeader::BuildFreeInodeEntryMap(void)
{
    InodeTableHeaderContent* content = GetContent();

    uint32_t size = content->inodeInUseBitmap.bits.size();

    for (uint32_t idx = 0; idx < size; idx++)
    {
        if (!content->inodeInUseBitmap.bits.test(idx))
        {
            freeInodeEntryIdxQ->push(idx);
        }
    }
}

std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>&
InodeTableHeader::GetInodeInUseBitmap(void)
{
    InodeTableHeaderContent* content = GetContent();
    return content->inodeInUseBitmap.bits;
}

uint32_t
InodeTableHeader::GetFreeInodeEntryIdx(void)
{
    uint32_t idx = freeInodeEntryIdxQ->front();
    freeInodeEntryIdxQ->pop();
    return idx;
}

std::vector<MetaFileExtent>
InodeTableHeader::GetFileExtentContent(void)
{
    std::vector<MetaFileExtent> extents;

    for (int i = 0; i < (int)content->allocExtentsList.size(); ++i)
    {
        if (content->allocExtentsList[i].GetCount() == 0)
            break;

        extents.push_back(content->allocExtentsList[i]);
    }

    return extents;
}

void
InodeTableHeader::SetFileExtentContent(std::vector<MetaFileExtent>& extents)
{
    assert(extents.size() <= content->allocExtentsList.size());

    for (int i = 0; i < (int)extents.size(); ++i)
    {
        content->allocExtentsList[i] = extents[i];
    }
}

size_t
InodeTableHeader::GetFileExtentContentSize(void)
{
    return MetaFsConfig::MAX_VOLUME_CNT;
}

bool
InodeTableHeader::Load(void)
{
    bool rc = OnVolumeMetaRegion<MetaRegionType, InodeTableHeaderContent>::Load();

    POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Load InodeTableHeader contents");

    if (true == rc)
    {
        _PrintLog();

        return true;
    }
    else
    {
        return false;
    }
}

bool
InodeTableHeader::Load(MetaStorageType media, MetaLpnType baseLPN, uint32_t idx,
    MetaLpnType pageCNT)
{
    bool rc = OnVolumeMetaRegion<MetaRegionType, InodeTableHeaderContent>::Load(
        media, baseLPN, idx, pageCNT);

    POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Load InodeTableHeader contents");

    if (true == rc)
    {
        _PrintLog();

        return true;
    }
    else
    {
        return false;
    }
}

bool
InodeTableHeader::Store(void)
{
    POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Store InodeTableHeader contents");

    _PrintLog();

    return OnVolumeMetaRegion<MetaRegionType, InodeTableHeaderContent>::Store();
}

bool
InodeTableHeader::Store(MetaStorageType media, MetaLpnType baseLPN, uint32_t idx,
    MetaLpnType pageCNT)
{
    POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Store InodeTableHeader contents");

    _PrintLog();

    return OnVolumeMetaRegion<MetaRegionType, InodeTableHeaderContent>::Store(
        media, baseLPN, idx, pageCNT);
}

void
InodeTableHeader::_PrintLog(void) const
{
    for (uint32_t i = 0; i < MetaFsConfig::MAX_VOLUME_CNT; ++i)
    {
        if (0 != content->allocExtentsList[i].GetCount())
        {
            POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
                "Allocated extent[{}]: start={}, count={}",
                i, content->allocExtentsList[i].GetStartLpn(),
                content->allocExtentsList[i].GetCount());
        }
    }
}
} // namespace pos
