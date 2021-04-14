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

#include "device_manager.h"
#include "src/device/base/ublock_device.h"

namespace pos
{
DeviceManager::DeviceManager(void)
{
}

DeviceManager::~DeviceManager()
{
}

void
DeviceManager::ScanDevs(void)
{
}

UblockSharedPtr
DeviceManager::GetDev(DeviceIdentifier& dev)
{
    return nullptr;
}

vector<UblockSharedPtr>
DeviceManager::GetDevs()
{
    vector<UblockSharedPtr> ret;
    return ret;
}

vector<DeviceProperty>
DeviceManager::ListDevs()
{
    vector<DeviceProperty> ret;
    return ret;
}

void
DeviceManager::AttachDevice(UblockSharedPtr dev)
{
}

int
DeviceManager::DetachDevice(DevUid sn)
{
    return 0;
}

void
DeviceManager::_PrepareDevices(void)
{
}

void
DeviceManager::_PrepareDevice(UblockSharedPtr dev)
{
}

void
DeviceManager::_PrepareMockDevice(UblockSharedPtr dev)
{
}

void
DeviceManager::_ReleaseDevice(UblockSharedPtr dev)
{
}

void
DeviceManager::HandleCompletedCommand(void)
{
}

void
DeviceManager::StartMonitoring()
{
}

void
DeviceManager::StopMonitoring()
{
}

vector<pair<string, string>>
DeviceManager::MonitoringState()
{
    vector<pair<string, string>> ret;
    return ret;
}

} // namespace pos
