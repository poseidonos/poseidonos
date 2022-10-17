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

#ifndef ARRAY_DEVICE_H_
#define ARRAY_DEVICE_H_

#include <atomic>
#include <string>
#include "src/include/i_array_device.h"

using namespace std;

namespace pos
{
class ArrayDevice : public IArrayDevice
{
public:
    ArrayDevice(UblockSharedPtr uBlock,
        ArrayDeviceState state = ArrayDeviceState::NORMAL,
        uint32_t dataIndex = 0);
    ~ArrayDevice(void) override;

    ArrayDeviceState GetState(void) override;
    void SetState(ArrayDeviceState state) override;

    UblockSharedPtr GetUblock(void) override;
    UBlockDevice* GetUblockPtr(void) override;
    void SetUblock(UblockSharedPtr uBlock) override;
    string GetName(void) override;
    string GetSerial(void) override;
    uint32_t GetDataIndex(void) { return dataIndex; }
    string PrevUblockInfo() { return prevUblockInfo; }

private:
    void _UpdateTrace(UblockSharedPtr uBlock);
    string prevUblockInfo = "";
    UblockSharedPtr uBlock;
    ArrayDeviceState state;
    uint32_t dataIndex;
};

} // namespace pos

#endif // ARRAY_DEVICE_H_
