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

#include <string>
#include <vector>
#include <unordered_map>

#include "src/metafs/common/meta_file_name.h"
#include "mf_inode_req.h"
#include "src/metafs/include/meta_file_extent.h"
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
    std::unordered_map<MetaLpnType, MetaFileExtent*>* journalmap;
};

static const uint32_t INODE_DATA_BYTE_SIZE = 4096 * 2;
static_assert(MetaFileInodeInfo::META_FILE_INODE_INFO_BYTE_SIZE == INODE_DATA_BYTE_SIZE);

union MetaFileInodeData
{
public:
    MetaFileInodeData(void);
    ~MetaFileInodeData(void);

    // NOTE: The defined variables will be read or written into meta storage as a data layout specified below
    // After it gets fixed, do not change the defined order of variables or add/remove any variables in the middle of it.
    union UBasic
    {
        struct S // DoNotAddAnyFieldInMiddle
        {
            // name                                size   address
            bool inUse;                         // 4      0....3
            uint32_t age;                       // 4      4....7
            uint64_t ctime;                     // 8      8....15
            uint32_t referenceCnt;              // 4      16...19
            FileDescriptorType fd;              // 4      20...23
            MetaFileName fileName;              // 128    24...151
            FileSizeType fileByteSize;          // 8      152..159
            FileSizeType dataChunkSize;         // 8      160..167
            MetaStorageIoProperty ioAttribute;  // 12     168..179
            uint8_t reserved0[4];               // 4      180..183
            uint32_t indexInInodeTable;         // 4      184..187
            uint16_t versionSignature;          // 2      188..189
            uint16_t version;                   // 2      190..191
            uint8_t reserved1[806];             // 806    192..997
            uint16_t pagemapCnt;                // 2      998..999
            MetaFileExtent pagemap[MetaFsConfig::MAX_PAGE_MAP_CNT];
                                                // 16*384 1000.7143
            uint8_t reserved2[1024];            // 1024   7144.8167
            uint64_t ctimeCopy;                 // 8      8168.8175
            uint32_t ageCopy;                   // 4      8176.8179
            uint8_t reserved3[12];              // 12     8180.8191
        } field;
        // do not define here anything
        uint8_t basicAll[1024];

        uint64_t GetLpnCount(void)
        {
            uint64_t ret = 0;
            for (int i = 0; i < field.pagemapCnt; ++i)
                ret += field.pagemap[i].GetCount();
            return ret;
        }

        bool CheckVersion(void)
        {
            if (MetaFsConfig::SIGNATURE_INODE_VERSION != field.versionSignature)
                return false;
            if (MetaFsConfig::CURRENT_INODE_VERSION != field.version)
                return false;
            return true;
        }

        bool CheckVerification(void)
        {
            if (field.age != field.ageCopy)
                return false;
            if (field.ctime != field.ctimeCopy)
                return false;
            return true;
        }
    } basic;

    union UJournal
    {
        struct S // DoNotAddAnyFieldInMiddle
        {
            JournalMDPageMap journal;
        } field;
        // do not define here anything
        uint8_t journalAll[512];
    } journal;

    union UExtra
    {
        struct // DoNotAddAnyFieldInMiddle
        {
        } field;

        uint8_t extraAll[512];
    } extra;
    // do not define anything below
    /////
    uint8_t all[INODE_DATA_BYTE_SIZE];
};
static_assert(INODE_DATA_BYTE_SIZE == sizeof(MetaFileInodeData));

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

    std::vector<MetaFileExtent> GetInodePageMap(void);

    MetaFileInodeData data;
    static_assert(sizeof(data) == INODE_DATA_BYTE_SIZE, "MetaFileInode size is overflow");
};
} // namespace pos
