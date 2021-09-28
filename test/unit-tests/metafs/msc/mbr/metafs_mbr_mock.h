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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "src/metafs/msc/mbr/metafs_mbr.h"

namespace pos
{
class MockMetaFsMBRContent : public MetaFsMBRContent
{
public:
    using MetaFsMBRContent::MetaFsMBRContent;
};

class MockMetaFsMBR : public MetaFsMBR
{
public:
    using MetaFsMBR::MetaFsMBR;
    MOCK_METHOD(bool, GetPORStatus, ());
    MOCK_METHOD(void, SetPORStatus, (bool isShutdownOff));
    MOCK_METHOD(void, InvalidMBRSignature, ());
    MOCK_METHOD(bool, Load, ());
    MOCK_METHOD(bool, Store, ());
    MOCK_METHOD(void, ResetContent, ());
    MOCK_METHOD(const MetaLpnType, GetLpnCntOfRegion, ());
    MOCK_METHOD(void, CreateMBR, ());
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem* mss));
    MOCK_METHOD(uint64_t, GetEpochSignature, (), (override));
    MOCK_METHOD(bool, IsValidMBRExist, ());
};

} // namespace pos
