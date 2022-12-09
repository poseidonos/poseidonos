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

#include "src/metafs/mim/mdpage.h"

namespace pos
{
class MockMDPage : public MDPage
{
public:
    using MDPage::MDPage;

    MOCK_METHOD(void, AttachControlInfo, ());
    MOCK_METHOD(void, Make, (const MetaLpnType metaLpn, const FileDescriptorType fd, const int arrayId, const uint64_t signature));
    MOCK_METHOD(bool, CheckValid, (const int arrayId), (const));
    MOCK_METHOD(bool, CheckFileMismatch, (const FileDescriptorType fd), (const));
    MOCK_METHOD(bool, CheckLpnMismatch, (const MetaLpnType srcLpn), (const));
    MOCK_METHOD(void, ClearCtrlInfo, ());
    MOCK_METHOD(uint8_t*, GetDataBuf, (), (const));
    MOCK_METHOD(size_t, GetDefaultDataChunkSize, (), (const));
    MOCK_METHOD(uint32_t, GetMfsSignature, (), (const));
};

} // namespace pos
