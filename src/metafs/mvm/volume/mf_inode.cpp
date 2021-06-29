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

#include "mf_inode.h"

#include "metafs_common_const.h"

namespace pos
{
MetaFileInodeData::MetaFileInodeData(void)
{
}

MetaFileInodeData::~MetaFileInodeData(void)
{
}

MetaFileInode::MetaFileInode(void)
{
    CleanupEntry();
}

MetaFileInode::~MetaFileInode(void)
{
}

bool
MetaFileInode::IsInUse(void)
{
    return data.basic.field.inUse;
}

void
MetaFileInode::SetInUse(bool inUse)
{
    data.basic.field.inUse = inUse;
}

FileSizeType
MetaFileInode::GetFileByteSize(void)
{
    return data.basic.field.fileByteSize;
}

void
MetaFileInode::BuildNewEntry(MetaFileInodeCreateReq& req, FileSizeType dataChunkSizeInMetaPage)
{
    data.basic.field.fd = req.fd;
    data.basic.field.fileName = req.fileName;
    data.basic.field.fileByteSize = req.fileByteSize;
    data.basic.field.dataChunkSize = dataChunkSizeInMetaPage;

    data.basic.field.ioAttribute.media = req.media;
    data.basic.field.ioAttribute.ioSpecfic = req.ioAttribute;
    data.basic.field.pagemap = *req.pageMap;
    data.basic.field.ctime = GetCurrDateTimestamp();

    data.basic.field.inUse = true;
}

void
MetaFileInode::CleanupEntry(void)
{
    memset(data.all, 0x0, INODE_DATA_BYTE_SIZE);
}

void
MetaFileInode::SetIndexInInodeTable(uint32_t entryIndex)
{
    data.basic.field.indexInInodeTable = entryIndex;
}

uint32_t
MetaFileInode::GetIndexInInodeTable(void)
{
    return data.basic.field.indexInInodeTable;
}

MetaStorageType
MetaFileInode::GetStorageType(void)
{
    return data.basic.field.ioAttribute.media;
}

void
MetaFileInode::SetMetaFileInfo(MetaStorageType mediaType, MetaFileInodeInfo& inodeInfo)
{
    inodeInfo.data.field.inUse = data.basic.field.inUse;
    inodeInfo.data.field.fd = data.basic.field.fd;
    memcpy(inodeInfo.data.field.fileName, data.basic.field.fileName.ToString().c_str(), data.basic.field.fileName.ToString().length());
    inodeInfo.data.field.fileByteSize = data.basic.field.fileByteSize;
    inodeInfo.data.field.dataChunkSize = data.basic.field.dataChunkSize;
    inodeInfo.data.field.dataLocation = mediaType;
    inodeInfo.data.field.fileProperty = data.basic.field.ioAttribute.ioSpecfic;
    inodeInfo.data.field.extentMap.baseMetaLpn = data.basic.field.pagemap.baseMetaLpn;
    inodeInfo.data.field.extentMap.pageCnt = data.basic.field.pagemap.pageCnt;
}

MetaFilePageMap
MetaFileInode::GetInodePageMap(void)
{
    return data.basic.field.pagemap;
}
} // namespace pos
