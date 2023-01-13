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
    static POS_EVENT_ID _FillVolumeList(VolumeList& volList, const std::string& arrayName,
        const int arrayID,const char *str);
    static std::string _CreateJsonFrom(VolumeList& volList);
    static int _ReadMetaFile(MetaFsFileIntf* file, char* buf);
    static int _WriteBuffer(MetaFsFileIntf* file, char* buf);
    static int _CloseFile(unique_ptr<MetaFsFileIntf> file);
    static std::unique_ptr<char[]> _AllocateBuffer(uint32_t size)
    {
        return std::unique_ptr<char[]>(new char[size]);
    }

    static const std::string FILE_NAME;
    static const uint32_t FILE_SIZE = 256 * 1024; // 256KB
};
} // namespace pos

#endif // VOLUME_META_INTF_H_
