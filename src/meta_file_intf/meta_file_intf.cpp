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

#include "src/meta_file_intf/meta_file_intf.h"

#include <fcntl.h>
#include <unistd.h>

#include <string>

#include "src/metafs/include/metafs_service.h"

namespace pos
{
MetaFileIntf::MetaFileIntf(void)
: fileName(""),
  arrayId(INT32_MAX),
  size(0),
  isOpened(false),
  fd(-1),
  volumeType(MetaVolumeType::SsdVolume),
  fileProperty(MetaFilePropertySet())
{
}

MetaFileIntf::MetaFileIntf(std::string fname, int arrayId, MetaVolumeType volumeType)
: MetaFileIntf()
{
    fileName = fname;
    this->arrayId = arrayId;
    this->volumeType = volumeType;
}

// LCOV_EXCL_START
MetaFileIntf::~MetaFileIntf(void)
{
}
// LCOV_EXCL_STOP

int
MetaFileIntf::IssueIO(MetaFsIoOpcode opType, uint64_t fileOffset,
    uint64_t length, char* buffer)
{
    int ret = 0;

    if (opType == MetaFsIoOpcode::Read)
    {
        ret = _Read(fd, fileOffset, length, buffer);
    }
    else if (opType == MetaFsIoOpcode::Write)
    {
        ret = _Write(fd, fileOffset, length, buffer);
    }

    return ret;
}

int
MetaFileIntf::AppendIO(MetaFsIoOpcode opType, uint64_t& offset,
    uint64_t length, char* buffer)
{
    int ret = 0;

    if (opType == MetaFsIoOpcode::Read)
    {
        ret = _Read(fd, offset, length, buffer);
    }
    else if (opType == MetaFsIoOpcode::Write)
    {
        ret = _Write(fd, offset, length, buffer);
    }

    offset += length;

    return ret;
}

int
MetaFileIntf::Open(void)
{
    isOpened = true;
    return 0;
}

int
MetaFileIntf::Close(void)
{
    isOpened = false;
    fd = -1;

    return 0;
}

bool
MetaFileIntf::IsOpened(void)
{
    return isOpened;
}

} // namespace pos
