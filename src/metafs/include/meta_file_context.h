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

#include <cstring>

#include "src/metafs/common/metafs_type.h"
#include "src/metafs/include/meta_file_extent.h"
#include "src/metafs/include/meta_storage_specific.h"
#include "src/metafs/include/mf_property.h"
#include "src/metafs/storage/mss.h"

namespace pos
{
class MetaFileContext
{
public:
    MetaFileContext(void)
    : extents(nullptr)
    {
        Reset();
    }

    void Reset(void)
    {
        isActivated = false;
        fileType = MetaFileType::General;
        storageType = MetaStorageType::Default;
        sizeInByte = 0;
        fileBaseLpn = 0;
        chunkSize = 0;
        extentsCount = 0;
        signature = 0;
        storage = nullptr;
        if (extents)
        {
            delete[] extents;
            extents = nullptr;
        }
    }

    void CopyExtentsFrom(const MetaFileExtent* const list, const int count)
    {
        assert(extents == nullptr);
        assert(list != nullptr);
        extents = new MetaFileExtent[count];
        memcpy(extents, list, sizeof(MetaFileExtent) * count);
    }

    // from MetaFileManager::CheckFileInActive()
    bool isActivated;
    MetaFileType fileType;
    MetaStorageType storageType;
    FileSizeType sizeInByte;
    MetaLpnType fileBaseLpn;
    FileSizeType chunkSize;
    int extentsCount;
    MetaFileExtent* extents;
    uint64_t signature;
    MetaStorageSubsystem* storage;
};
} // namespace pos
