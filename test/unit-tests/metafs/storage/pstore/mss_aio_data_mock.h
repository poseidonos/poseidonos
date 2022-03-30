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

#include "src/metafs/storage/pstore/mss_aio_data.h"

namespace pos
{
class MockMssAioData : public MssAioData
{
public:
    using MssAioData::MssAioData;
    MOCK_METHOD(void, Init, (const int arrayId, const MetaStorageType media,
        const MetaLpnType metaLpn, const MetaLpnType lpnCnt, void* buf,
        const uint32_t mpioId, const uint32_t tagId, const FileSizeType offset));
    MOCK_METHOD(int, GetArrayId, (), (const));
    MOCK_METHOD(MetaStorageType, GetStorageType, (), (const));
    MOCK_METHOD(MetaLpnType, GetMetaLpn, (), (const));
    MOCK_METHOD(MetaLpnType, GetLpnCount, (), (const));
    MOCK_METHOD(void*, GetBuffer, ());
    MOCK_METHOD(int, GetError, (), (const));
    MOCK_METHOD(void, SetError, (const int err));
    MOCK_METHOD(bool, GetErrorStopState, (), (const));
    MOCK_METHOD(void, SetErrorStopState, (const bool state));
    MOCK_METHOD(uint32_t, GetMpioId, (), (const));
    MOCK_METHOD(uint32_t, GetTagId, (), (const));
    MOCK_METHOD(FileSizeType, GetOffset, (), (const));
};

} // namespace pos
