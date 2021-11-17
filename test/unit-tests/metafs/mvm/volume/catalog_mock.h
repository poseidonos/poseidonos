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

#include "src/metafs/mvm/volume/catalog.h"

namespace pos
{
class MockCatalog : public Catalog
{
public:
    using Catalog::Catalog;
    MOCK_METHOD(void, Create, (MetaLpnType maxVolumeLpn, uint32_t maxFileNumSupport));
    MOCK_METHOD(void, RegisterRegionInfo, (MetaRegionType regionType, MetaLpnType baseLpn, MetaLpnType maxLpn));
    MOCK_METHOD(bool, CheckValidity, ());
    MOCK_METHOD(bool, Store, ());
    MOCK_METHOD(bool, Store, (MetaStorageType targetMedia, MetaLpnType startLPN,
        uint32_t idx, MetaLpnType pageCNT));
    MOCK_METHOD(bool, Load, ());
    MOCK_METHOD(bool, Load, (MetaStorageType targetMedia, MetaLpnType startLPN,
        uint32_t idx, MetaLpnType pageCNT));
};

} // namespace pos
