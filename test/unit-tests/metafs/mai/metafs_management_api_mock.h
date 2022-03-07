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
#include <vector>

#include "src/metafs/mai/metafs_management_api.h"

namespace pos
{
class MockMetaFsManagementApi : public MetaFsManagementApi
{
public:
    using MetaFsManagementApi::MetaFsManagementApi;
    MOCK_METHOD(POS_EVENT_ID, InitializeSystem, (int arrayId, MetaStorageInfoList* mediaInfoList));
    MOCK_METHOD(POS_EVENT_ID, CloseSystem, (int arrayId));
    MOCK_METHOD(uint64_t, GetEpochSignature, ());
    MOCK_METHOD(MetaFsStorageIoInfoList&, GetAllStoragePartitionInfo, ());
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, ());
    MOCK_METHOD(POS_EVENT_ID, LoadMbr, (bool& isNPOR));
    MOCK_METHOD(bool, CreateMbr, ());
    MOCK_METHOD(void, SetStatus, (bool isNormal));
    MOCK_METHOD(bool, IsValidVolume, (MetaVolumeType volumeType), (override));
};

} // namespace pos
