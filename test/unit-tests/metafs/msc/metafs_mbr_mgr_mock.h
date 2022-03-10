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
#include <memory>
#include <string>
#include <vector>

#include "src/metafs/msc/metafs_mbr_mgr.h"

namespace pos
{
class MockMetaFsMBRManager : public MetaFsMBRManager
{
public:
    using MetaFsMBRManager::MetaFsMBRManager;
    MOCK_METHOD(void, Init, (MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn), (override));
    MOCK_METHOD(void, Bringup, (), (override));
    MOCK_METHOD(bool, SaveContent, (), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(void, Finalize, (), (override));
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem* mss));

    MOCK_METHOD(bool, IsValidMBRExist, ());
    MOCK_METHOD(uint64_t, GetEpochSignature, (), (override));
    MOCK_METHOD(bool, LoadMBR, ());
    MOCK_METHOD(bool, CreateMBR, ());
    MOCK_METHOD(void, RegisterVolumeGeometry, (std::shared_ptr<MetaStorageInfo> mediaInfo));
    MOCK_METHOD(MetaFsStorageIoInfoList&, GetAllStoragePartitionInfo, ());

    MOCK_METHOD(void, SetPowerStatus, (bool isShutDownOff));
    MOCK_METHOD(bool, GetPowerStatus, ());
    MOCK_METHOD(void, InvalidMBR, ());
};

} // namespace pos
