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

#include "src/metafs/mvm/volume/inode_manager.h"
#include "src/metafs/mvm/volume/inode_deleter.h"

namespace pos
{
InodeDeleter::InodeDeleter(InodeManager* _inodeMgr)
: inodeMgr(_inodeMgr)
{
}

InodeDeleter::~InodeDeleter(void)
{
}

std::pair<FileDescriptorType, POS_EVENT_ID>
InodeDeleter::Delete(MetaFsFileControlRequest& reqMsg)
{
    MetaLpnType totalLpnCount = 0;
    std::vector<MetaFileExtent> extents;
    FileDescriptorType fd = inodeMgr->LookupDescriptorByName(*reqMsg.fileName);
    MetaLpnType count = inodeMgr->GetExtent(fd, extents);
    assert(count > 0);

    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "DeleteFileInode, fd: {}, fileName: {}",
        reqMsg.fd, *reqMsg.fileName);

    for (auto& extent : extents)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "target extent, startLpn: {}, lpnCount: {}",
            extent.GetStartLpn(), extent.GetCount());
    }

    MetaFileInode& inode = inodeMgr->GetFileInode(fd);
    uint32_t entryIdx = inode.GetIndexInInodeTable();

    inode.SetInUse(false);
    inodeMgr->inodeHdr->ClearInodeInUse(entryIdx);

    inodeMgr->fd2InodeMap.erase(fd);

    totalLpnCount = 0;
    for (auto& extent : extents)
    {
        inodeMgr->extentAllocator->AddToFreeList(extent.GetStartLpn(), extent.GetCount());

        POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "[Metadata File] Release an extent, startLpn={}, count={}",
            extent.GetStartLpn(), extent.GetCount());
            totalLpnCount += extent.GetCount();
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Metadata File] Delete volType={}, fd={}, fileName={}, totalLpnCnt={}",
        (int)inodeMgr->volumeType, fd, *reqMsg.fileName, totalLpnCount);

    inodeMgr->fdAllocator->Free(*reqMsg.fileName, fd);

    inodeMgr->extentAllocator->PrintFreeExtentsList();

    std::vector<pos::MetaFileExtent> usedExtentsInVolume =
                            inodeMgr->extentAllocator->GetAllocatedExtentList();
    inodeMgr->inodeHdr->SetFileExtentContent(usedExtentsInVolume);

    if (true != inodeMgr->SaveContent())
    {
        return std::make_pair(0, POS_EVENT_ID::MFS_META_SAVE_FAILED);
    }

    return std::make_pair(fd, POS_EVENT_ID::SUCCESS);
}
} // namespace pos
