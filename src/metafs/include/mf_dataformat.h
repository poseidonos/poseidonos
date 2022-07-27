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

#include "metafs_common.h"
#include "src/metafs/include/meta_storage_specific.h"
#include "src/metafs/include/mf_property.h"
#include "src/metafs/include/meta_file_extent.h"

namespace pos
{
struct MetaFileInfoDumpCxt
{
public:
    std::string fileName;
    FileDescriptorType fd;
    FileSizeType size;
    uint64_t ctime;
    MetaLpnType lpnBase;
    MetaLpnType lpnCount;
    std::string location;
};

class MetaFileInodeInfo
{
public:
    MetaFileInodeInfo(void)
    {
        memset(data.all, 0, META_FILE_INODE_INFO_BYTE_SIZE);
    }

    static const uint32_t META_FILE_INODE_INFO_BYTE_SIZE = 4096 * 2;

    union UData {
        UData(void)
        {
        }
        struct S
        {
            bool inUse;
            FileDescriptorType fd;
            char fileName[128]; // Change this to MAX_FILE_NAME_LENGTH or string *
            FileSizeType fileByteSize;
            FileSizeType dataChunkSize;
            MetaStorageType dataLocation;
            MetaFilePropertySet fileProperty;
            uint16_t extentCnt;
            MetaFileExtent extentMap[MetaFsConfig::MAX_PAGE_MAP_CNT];
        } field;
        uint8_t all[META_FILE_INODE_INFO_BYTE_SIZE];
    } data;
};

class MetaFileInodeDumpCxt
{
public:
    MetaFileInodeInfo inodeInfo;
    uint32_t inodeSize = MetaFileInodeInfo::META_FILE_INODE_INFO_BYTE_SIZE;
};
} // namespace pos
