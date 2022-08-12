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

#include "metafs_common.h"
#include "src/include/pos_event_id.h"
#include <string>
#include <array>
#include <algorithm>

class MetaFileName
{
public:
    MetaFileName(void)
    {
        memset(str.data(), 0, MAX_FILE_NAME_LENGTH + 1);
    }

    MetaFileName(const MetaFileName& original)
    {
        std::copy(std::begin(original.str), std::end(original.str), std::begin(str));
    }

    MetaFileName(const string& fileName)
    {
        if (!_IsValidFileName(fileName))
            assert(false);

        strncpy(str.data(), fileName.c_str(), MAX_FILE_NAME_LENGTH + 1);
    }

    MetaFileName(const string* fileName) : MetaFileName(*fileName)
    {
        if (!_IsValidPtr(fileName) || !_IsValidFileName(*fileName))
            assert(false);
    }

    MetaFileName&
    operator=(const std::string& fileName)
    {
        if (!_IsValidFileName(fileName))
            assert(false);

        strncpy(str.data(), fileName.c_str(), MAX_FILE_NAME_LENGTH + 1);
        return *this;
    }

    MetaFileName& operator=(const std::string* fileName)
    {
        if (!_IsValidPtr(fileName) || !_IsValidFileName(*fileName))
            assert(false);

        return *this = *fileName;
    }

    bool operator==(const std::string& fileName)
    {
        return (0 == fileName.compare(ToString()));
    }

    bool operator!=(const std::string& fileName)
    {
        return !(*this == fileName);
    }

    bool operator==(MetaFileName& fileName)
    {
        return (0 == fileName.ToString().compare(ToString()));
    }

    bool operator!=(MetaFileName& fileName)
    {
        return !(*this == fileName);
    }

    std::string ToString(void)
    {
        return std::string(str.data());
    }

    const char* ToChar(void)
    {
        return reinterpret_cast<const char*>(&str);
    }

    size_t GetLength(void)
    {
        return ToString().length();
    }

    static const size_t MAX_FILE_NAME_LENGTH = 127;

private:
    bool _IsValidPtr(const std::string* fileName)
    {
        if (nullptr == fileName)
        {
            POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
                "The file name is nullptr");
            return false;
        }
        return true;
    }

    bool _IsValidFileName(const std::string& fileName)
    {
        if (fileName.length() > MAX_FILE_NAME_LENGTH)
        {
            const size_t MAX_SIZE = MAX_FILE_NAME_LENGTH;
            POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
                "The file name({}) is too long (len: {}, max_allowed: {})",
                fileName, fileName.length(), MAX_SIZE);
            return false;
        }
        return true;
    }

    std::array<char, MAX_FILE_NAME_LENGTH + 1> str;
};
