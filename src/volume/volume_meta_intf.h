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

#ifndef VOLUME_META_INTF_H_
#define VOLUME_META_INTF_H_

#include <memory>
#include <string>

#include "src/metafs/metafs_file_intf.h"
#include "src/volume/volume_list.h"

namespace pos
{
struct BufferDeleter
{
    void operator()(char* const ptr) const
    {
        free(ptr);
    }
};

class VolumeMetaIntf
{
public:
    // test file is used by ut
    static int LoadVolumes(VolumeList& volList, const std::string& arrayName,
        const int arrayID, std::unique_ptr<MetaFsFileIntf> testFile = nullptr);
    // test file is used by ut
    static int SaveVolumes(VolumeList& volList, const std::string& arrayName,
        const int arrayID, std::unique_ptr<MetaFsFileIntf> testFile = nullptr);

private:
    static int _CloseFile(unique_ptr<MetaFsFileIntf> file);
    static std::unique_ptr<char, BufferDeleter> _GetBuffer(uint32_t size)
    {
        std::unique_ptr<char, BufferDeleter> buf((char*)malloc(size));
        return buf;
    }
};
} // namespace pos

#endif // VOLUME_META_INTF_H_
