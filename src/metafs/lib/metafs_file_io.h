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

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>

namespace pos
{
class MetaFsFileIo
{
public:
    MetaFsFileIo(void) = delete;
    explicit MetaFsFileIo(const std::string& fileName)
    : fileName(fileName)
    {
        file.open(fileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    }
    virtual ~MetaFsFileIo(void)
    {
        file.close();
    }
    virtual bool Write(void* const buf, const size_t byteOffset, const size_t byteSize)
    {
        file.clear();
        file.seekp(byteOffset, file.beg);
        if (!file.good())
        {
            return false;
        }
        file.write(static_cast<char*>(buf), byteSize);
        if (!file.good())
        {
            return false;
        }
        return true;
    }
    virtual bool Read(void* buf, const size_t byteOffset, const size_t byteSize)
    {
        file.clear();
        file.seekg(byteOffset);
        if (!file.good())
        {
            std::memset(buf, 0, byteSize);
            return true;
        }
        file.read((char*)buf, byteSize);
        if (!file.good())
        {
            std::memset(buf, 0, byteSize);
        }
        return true;
    }

private:
    std::string fileName;
    std::fstream file;
};
} // namespace pos
