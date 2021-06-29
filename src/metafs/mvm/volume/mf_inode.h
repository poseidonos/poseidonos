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

#include "src/metafs/common/meta_file_name.h"
#include "mf_inode_req.h"
#include "mf_pagemap.h"
#include "metafs_common.h"

namespace pos
{
class MetaStorageIoProperty
{
public:
    MetaStorageType media;
    MetaFilePropertySet ioSpecfic;
};

class JournalMDPageMap
{
public:
    std::unordered_map<MetaLpnType, MetaFilePageMap*>* journalmap;
};

static const uint32_t INODE_DATA_BYTE_SIZE = 4096;
static_assert(MetaFileInodeInfo::META_FILE_INODE_INFO_BYTE_SIZE == INODE_DATA_BYTE_SIZE);
union MetaFileInodeData {
public:
    MetaFileInodeData(void);
    ~MetaFileInodeData(void);

    // NOTE: The defined variables will be read or written into meta storage as a data layout specified below
    // After it gets fixed, do not change the defined order of variables or add/remove any variables in the middle of it.
    union UBasic {
        struct S // DoNotAddAnyFieldInMiddle
        {
            bool inUse;
            uint64_t ctime;
            uint32_t referenceCnt;
            FileDescriptorType fd;
            MetaFileName fileName;
            FileSizeType fileByteSize;
            FileSizeType dataChunkSize;
            MetaStorageIoProperty ioAttribute;
            MetaFilePageMap pagemap;
            uint32_t indexInInodeTable;
        } field;
        // do not define here anything
        uint8_t basicAll[1024];
    } basic;

    union UJournal {
        struct S // DoNotAddAnyFieldInMiddle
        {
            JournalMDPageMap journal;
        } field;
        // do not define here anything
        uint8_t journalAll[512];
    } journal;

    union UExtra {
        struct // DoNotAddAnyFieldInMiddle
        {
        } field;

        uint8_t extraAll[512];
    } extra;
    // do not define anything below
    /////
    uint8_t all[INODE_DATA_BYTE_SIZE];
};

// meta file inode entry. Each file has meta i-node entry.
// all information specified in this context will be preserved persistently.
// Note: please do not define any runtime variables in this context
class MetaFileInode
{
public:
    MetaFileInode(void);
    ~MetaFileInode(void);

    bool IsInUse(void);
    void SetInUse(bool inUse);
    FileSizeType GetFileByteSize(void);
    MetaStorageType GetStorageType(void);
    void BuildNewEntry(MetaFileInodeCreateReq& req, FileSizeType dataChunkSizeInMetaPage);
    void CleanupEntry(void);
    void SetIndexInInodeTable(uint32_t entryIndex);
    uint32_t GetIndexInInodeTable(void);
    void SetMetaFileInfo(MetaStorageType mediaType, MetaFileInodeInfo& inodeInfo);

    MetaFilePageMap GetInodePageMap(void);

    MetaFileInodeData data;
    static_assert(sizeof(data) == INODE_DATA_BYTE_SIZE, "MetaFileInode size is overflow");
};
} // namespace pos
