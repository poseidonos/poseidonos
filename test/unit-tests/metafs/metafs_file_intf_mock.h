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

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/metafs_file_intf.h"

namespace pos
{
class MockMetaFsFileIntf : public MetaFsFileIntf
{
public:
    using MetaFsFileIntf::MetaFsFileIntf;
    MOCK_METHOD(int, Create, (uint64_t fileSize), (override));
    MOCK_METHOD(bool, DoesFileExist, (), (override));
    MOCK_METHOD(int, Delete, (), (override));
    MOCK_METHOD(uint64_t, GetFileSize, (), (override));
    MOCK_METHOD(int, AsyncIO, (AsyncMetaFileIoCtx * ctx), (override));
    MOCK_METHOD(int, CheckIoDoneStatus, (void* data), (override));
    MOCK_METHOD(int, Open, (), (override));
    MOCK_METHOD(int, Close, (), (override));
    MOCK_METHOD(int, _Read, (int fd, uint64_t fileOffset, uint64_t length, char* buffer), (override));
    MOCK_METHOD(int, _Write, (int fd, uint64_t fileOffset, uint64_t length, char* buffer), (override));
};

} // namespace pos
