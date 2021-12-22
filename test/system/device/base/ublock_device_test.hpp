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

#ifndef UBLOCK_DEVICE_TEST_HPP_
#define UBLOCK_DEVICE_TEST_HPP_

#include <cstdint>
#include <thread>
#include <vector>

#include "src/device/base/device_driver.h"
#include "src/device/base/ublock_device.h"
#include "src/bio/ubio.h"

namespace pos
{
class UBlockDeviceTest
{
public:
    UBlockDeviceTest(const char* deviceName, DeviceDriver* inputDeviceDriver,
        uint32_t ioContextCount);
    virtual ~UBlockDeviceTest(void);

    static void PrintUsage(void);
    static const unsigned int IO_PAGE_COUNT = 64; // 4KB per page

    bool TestScanDevs(void);
    bool TestOpenClose(void);
    bool TestAsyncIO(void);

    bool TestOpen(void);
    bool TestClose(void);

    void PrepareThreadRegister(void);
    void RegisterThread(void);
    void WaitThreadRegisterDone(void);

    void PrepareThreadUnregister(void);
    void UnregisterThread(void);
    void WaitThreadUnregisterDone(void);

    void WaitOpenDone(void);
    void WaitCloseDone(void);

    void SetTestFailed(void);
    void SetThreadTestDone(uint32_t threadIndex);

private:
    virtual uint32_t _SubmitIO(UbioSmartPtr ioToSubmit, uint32_t threadIndex);
    virtual uint32_t _CheckIOCompletion(UblockSharedPtr dev, uint32_t threadIndex);

    void _PrintDevice(void);

    void _WaitThreadTestDone(uint32_t threadIndex);
    bool _RequestAndVerifyAsyncIO(UbioSmartPtr ubioRead, UbioSmartPtr ubioWrite,
        uint32_t threadIndex);
    UbioSmartPtr _GenerateUbio(void);
    bool _ReleaseUbio(UbioSmartPtr ubio);
    void _GeneratePattern(void* mem, unsigned int sizeInBytes);
    bool _VerifyPattern(
        const void* dst, const void* src, unsigned int sizeInBytes);

    static void _OpenCloseTestThread(UBlockDeviceTest* test);
    static void _ASyncIOTestThread(UBlockDeviceTest* test, uint32_t threadIndex);

    const char* targetDeviceName;
    uint32_t threadCount;
    std::atomic<bool>* threadTestDoneList;
    std::atomic<bool> registerThreadDone;
    std::atomic<bool> unregisterThreadDone;
    std::atomic<bool> openDone;
    std::atomic<bool> closeDone;
    std::atomic<bool> overallTestSuccessful;
    std::mutex unregisterLock;
    std::vector<UblockSharedPtr> deviceList;
    std::vector<std::thread*> threadList;
    DeviceDriver* deviceDriver;
};

} // namespace pos
#endif // UBLOCK_DEVICE_TEST_HPP_
