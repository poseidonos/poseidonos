/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include <iostream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/bio/volume_io.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/meta_service/i_meta_updater.h"

#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/io/general_io/rba_state_manager_mock.h"
#include "test/unit-tests/allocator_service/allocator_service_mock.h"
#include "test/integration-tests/framework/write_tester/volume_io_fake.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/io/general_io/translator_mock.h"
#include "test/integration-tests/allocator/allocator_it_common.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Matcher;

namespace pos
{
class WriteTester
{
public:
    WriteTester(void);
    ~WriteTester(void);

    void WriteIo(FakeVolumeIo* newVolumeIo, uint32_t numBlks);
    void UpdateAddress(void);

private:
    VirtualBlks InitVsaRange(uint32_t numBlks);
    NiceMock<MockIBlockAllocator>* InitBlockAllocator(VirtualBlks vsaRange);
    NiceMock<MockTranslator>* InitTranslator(VirtualBlkAddr vsa, uint32_t arrayId, uint32_t numBlks);

    NiceMock<MockAllocatorService>* mockAllocatorService;
    NiceMock<MockRBAStateManager>* mockRBAStateManager;
    
    uint32_t curStripeId;
    uint32_t curBlkOffset;
    uint32_t curLba;
};
} // namespace pos