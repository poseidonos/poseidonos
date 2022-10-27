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

#include <gtest/gtest.h>
#include "src/cpu_affinity/cpu_set_generator.h"
#include "src/array_components/array_components.h"
#include "test/unit-tests/utils/mock_builder.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/array_components/array_components_mock.h"
#include "test/unit-tests/mbr/abr_manager_mock.h"
#include "test/unit-tests/array/array_mock.h"
#include "test/unit-tests/state/state_manager_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_client_mock.h"


using ::testing::NiceMock;
using ::testing::Return;

namespace pos {

MockAffinityManager
BuildDefaultAffinityManagerMock(void)
{
    // the actual value doesn't matter since the UTs using this mock wouldn't rely on CPU counts anyway.
    int MOCK_CPU_CORE_COUNT = 8;
    CpuSetArray cpuSetArray;
    MockAffinityManager mockAffMgr(MOCK_CPU_CORE_COUNT, cpuSetArray);  // The parent's constructor will copy from "cpuSetArray", so we're okay to use a local CpuSetArray instance.
    return mockAffMgr;  // we're okay to return the local instance of MockAffinityManager (vs. its pointer) because the caller can use copy-constructor and this mock doesn't point to any heap objects.
}

// the return type equals UblockSharedPtr
std::shared_ptr<UBlockDevice>
BuildMockUBlockDevice(const char* devName, const std::string& SN)
{
    MockUBlockDevice* rawPtr = new MockUBlockDevice(devName, 1024, nullptr);
    EXPECT_CALL(*rawPtr, GetName).WillRepeatedly(Return(devName));
    EXPECT_CALL(*rawPtr, GetSN).WillRepeatedly(Return(SN));
    return shared_ptr<UBlockDevice>(rawPtr);
}

std::shared_ptr<MockArray>
BuildMockArray(std::string arrayName)
{
    return std::make_shared<MockArray>(arrayName, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr);
}

std::shared_ptr<MockArrayComponents>
BuildMockArrayComponents(std::string arrayName, StateManager* stateManager)
{
    return std::make_shared<MockArrayComponents>(arrayName, nullptr, nullptr,
        stateManager, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr);
}

std::shared_ptr<MockArrayComponents>
BuildMockArrayComponents(std::string arrayName, Array* array)
{
    NiceMock<MockStateManager>* mockStateMgr = new NiceMock<MockStateManager>();
    return std::make_shared<MockArrayComponents>(arrayName, nullptr, nullptr,
        mockStateMgr, nullptr, array, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr);
}

MockArrayComponents*
NewMockArrayComponents(std::string arrayName)
{
    NiceMock<MockStateManager>* mockStateMgr = new NiceMock<MockStateManager>();
    return new MockArrayComponents(arrayName, nullptr, nullptr,
        mockStateMgr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr);
}

std::map<std::string, ArrayComponents*>
BuildArrayComponentsMap(std::string arrayName, ArrayComponents* arrayComponents)
{
    auto arrayCompMap = BuildArrayComponentsMap();
    arrayCompMap.emplace(arrayName, arrayComponents);
    return arrayCompMap;
}

std::map<std::string, ArrayComponents*>
BuildArrayComponentsMap(void)
{
    return map<std::string, ArrayComponents*>();
}

std::shared_ptr<MockAbrManager>
BuildMockAbrManager(void)
{
    return std::make_shared<MockAbrManager>(nullptr);
}

std::shared_ptr<MockTelemetryClient>
BuildMockTelemetryClient(void)
{
    return std::make_shared<MockTelemetryClient>();
}

}  // namespace pos
