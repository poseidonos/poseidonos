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

#pragma once

#include <string>
#include <vector>

#include "array_device.h"
#include "src/array_models/dto/device_set.h"

using namespace std;

namespace pos
{
class DeviceManager;

class ArrayDeviceApi
{
public:
    static ArrayDevice* FindDevByName(string devName, const vector<ArrayDevice*>& devs);
    static ArrayDevice* FindDevBySn(string serialNumber, const vector<ArrayDevice*>& devs);
    static vector<IArrayDevice*> ConvertToInterface(const vector<ArrayDevice*>& devs);
    static vector<UblockSharedPtr> ConvertToUblock(const vector<ArrayDevice*>& devs);
    static vector<ArrayDevice*> ExtractDevicesByState(ArrayDeviceState state, const vector<ArrayDevice*>& devs);
    static vector<ArrayDevice*> ExtractDevicesByType(ArrayDeviceType type, const vector<ArrayDevice*>& devs);
    static vector<ArrayDevice*> ExtractDevices(const vector<ArrayDevice*>& devs);
    static vector<ArrayDevice*> ExtractDevicesByTypeAndState(ArrayDeviceType type, ArrayDeviceState state, const vector<ArrayDevice*>& devs);
    static uint64_t GetMinimumCapacity(const vector<ArrayDevice*>& devs);
    static int ImportInspection(const vector<ArrayDevice*>& devs, string raidType);
};

} // namespace pos
