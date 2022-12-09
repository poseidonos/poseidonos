/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/metafs/config/metafs_config_manager.h"

namespace pos
{
class MockMetaFsConfigManager : public MetaFsConfigManager
{
public:
    using MetaFsConfigManager::MetaFsConfigManager;
    MOCK_METHOD(bool, Init, ());
    MOCK_METHOD(size_t, GetMioPoolCapacity, (), (const));
    MOCK_METHOD(size_t, GetMpioPoolCapacity, (), (const));
    MOCK_METHOD(size_t, GetWriteMpioCacheCapacity, (), (const));
    MOCK_METHOD(bool, IsDirectAccessEnabled, (), (const));
    MOCK_METHOD(size_t, GetTimeIntervalInMillisecondsForMetric, (), (const));
    MOCK_METHOD(size_t, GetSamplingSkipCount, (), (const));
    MOCK_METHOD(size_t, GetWrrCountSpecialPurposeMap, (), (const));
    MOCK_METHOD(size_t, GetWrrCountJournal, (), (const));
    MOCK_METHOD(size_t, GetWrrCountMap, (), (const));
    MOCK_METHOD(size_t, GetWrrCountGeneral, (), (const));
    MOCK_METHOD(std::vector<int>, GetWrrWeight, (), (const));
    MOCK_METHOD(bool, IsRocksdbEnabled, (), (const));
    MOCK_METHOD(bool, IsSupportingNumaDedicatedScheduling, (), (const));
    MOCK_METHOD(void, SetNumberOfScheduler, (const int count));
    MOCK_METHOD(int, GetNumberOfScheduler, (), (const));
    MOCK_METHOD(void, SetIgnoreNumaDedicatedScheduling, (const bool ignore));
    MOCK_METHOD(bool, NeedToIgnoreNumaDedicatedScheduling, (), (const));
};
} // namespace pos
