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

#ifndef MOCK_DEVICE_MANAGER_H_
#define MOCK_DEVICE_MANAGER_H_

#include <string>
#include <vector>

#include "device_manager.h"

namespace ibofos
{
using ::testing::_;
using ::testing::Return;

class MockDeviceManager : public DeviceManager
{
public:
    MOCK_METHOD(void, ScanDevs, ());
    MOCK_METHOD(UBlockDevice*, GetDev, (DevName));
    MOCK_METHOD(UBlockDevice*, GetDev, (DevUid));
    MOCK_METHOD(vector<UBlockDevice*>, GetDevs, ());
    MOCK_METHOD(vector<DeviceProperty>, ListDevs, ());
    MOCK_METHOD(void, AttachDevice, (UBlockDevice*));
    MOCK_METHOD(void, DetachDevice, (string));
    MOCK_METHOD(int, RemoveDevice, (UBlockDevice*));
    MOCK_METHOD(void, OpenAllDevices, ());
    MOCK_METHOD(void, RegisterToAllDevices, ());
    MOCK_METHOD(void, HandleCompletedCommand, ());
    MOCK_METHOD(void, PrepareDevice, (string));
    MOCK_METHOD(void, StartMonitoring, ());
    MOCK_METHOD(void, StopMonitoring, ());
};

} // namespace ibofos
#endif // MOCK_DEVICE_MANAGER_H_
