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

#include "src/bio/ubio.h"

namespace pos
{
class DeviceContext;
class DeviceDriver;
class Ubio;

class NVMeTest
{
public:
    NVMeTest(const char* deviceName, DeviceDriver* inputDeviceDriver);
    virtual ~NVMeTest(void);

    static void PrintUsage(void);
    void TestScanDevs(void);
    bool TestOpenClose(void);
    bool TestAsyncIO(void);
    bool TestOpen(void);
    bool TestClose(void);

    static const unsigned int IO_PAGE_COUNT = 64; // 4KB per page

protected:
    NVMeTest(DeviceDriver* inputDeviceDriver);

    DeviceContext* deviceContext;
    DeviceDriver* deviceDriver;

private:
    virtual void _PrintDevice(void);
    virtual bool _TestDeviceContextInitialized(void);

    virtual uint32_t _SubmitIO(UbioSmartPtr ioToSubmit);
    virtual uint32_t _CheckIOCompletion(void);

    bool _RequestAndVerifyAsyncIO(UbioSmartPtr ubioRead, UbioSmartPtr ubioWrite);
    UbioSmartPtr _GenerateUbio(void);
    bool _ReleaseUbio(UbioSmartPtr ubio);
    void _GeneratePattern(void* mem, unsigned int sizeInBytes);
    bool _VerifyPattern(
        const void* dst, const void* src, unsigned int sizeInBytes);
};
} // namespace pos
