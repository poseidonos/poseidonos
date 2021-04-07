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

#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>

#include "src/array/device/array_device.h"
#include "src/include/memory.h"
#include "ublock_device_test.hpp"

namespace ibofos
{
UBlockDeviceTest::UBlockDeviceTest(
    const char* deviceName, DeviceDriver* inputDeviceDriver,
    uint32_t ioContextCount)
: targetDeviceName(deviceName),
  threadCount(ioContextCount),
  registerThreadDone(false),
  unregisterThreadDone(false),
  openDone(false),
  closeDone(false),
  overallTestSuccessful(true),
  deviceDriver(inputDeviceDriver)
{
    threadTestDoneList = new std::atomic<bool>[threadCount];
}

UBlockDeviceTest::~UBlockDeviceTest()
{
    delete threadTestDoneList;
}

bool
UBlockDeviceTest::TestScanDevs(void)
{
    bool testSuccessful = false;
    UBlockDevice* targetDevice = nullptr;
    int scannedDeviceCount = deviceDriver->ScanDevs(&deviceList);

    std::cout << "Scanned "
              << scannedDeviceCount << " Devices in total." << std::endl;

    _PrintDevice();

    if (nullptr != targetDeviceName)
    {
        for (UBlockDevice* dev : deviceList)
        {
            if (0 == strncmp(targetDeviceName, dev->GetName(), strlen(targetDeviceName)))
            {
                targetDevice = dev;
                testSuccessful = true;
            }
            else
            {
                delete dev;
            }
        }

        deviceList.clear();

        if (testSuccessful)
        {
            deviceList.push_back(targetDevice);
            std::cout << "Target device found: ";
        }
        else
        {
            std::cout << "Target device NOT found: ";
        }

        std::cout << targetDeviceName << std::endl;
    }

    if (0 < deviceList.size())
    {
        testSuccessful = true;
    }

    return testSuccessful;
}

bool
UBlockDeviceTest::TestOpenClose(void)
{
    bool testSuccessful = false;

    do
    {
        overallTestSuccessful = true;
        openDone = false;
        closeDone = false;
        threadList.clear();

        for (uint32_t threadIndex = 0; threadIndex < threadCount; threadIndex++)
        {
            registerThreadDone = false;

            std::thread* testThread = new std::thread(&_OpenCloseTestThread, this);
            threadList.push_back(testThread);

            WaitThreadRegisterDone();
        }

        testSuccessful = TestOpen();
        if (false == testSuccessful)
        {
            break;
        }

        openDone = testSuccessful;

        testSuccessful = TestClose();
        if (false == testSuccessful)
        {
            break;
        }

        closeDone = testSuccessful;

        for (std::thread* testThread : threadList)
        {
            testThread->join();
        }

        testSuccessful = overallTestSuccessful;
    } while (false);

    for (std::thread* testThread : threadList)
    {
        delete testThread;
    }

    if (false != testSuccessful)
    {
        std::cout << "OpenClose Test Succeeded: " << std::endl;
    }
    else
    {
        std::cout << "OpenClose Test Failed: " << std::endl;
    }

    return testSuccessful;
}

void
UBlockDeviceTest::_OpenCloseTestThread(UBlockDeviceTest* test)
{
    test->RegisterThread();

    test->WaitOpenDone();

    test->WaitCloseDone();

    test->UnregisterThread();
}

bool
UBlockDeviceTest::TestOpen(void)
{
    bool testSuccessful = false;

    for (UBlockDevice* dev : deviceList)
    {
        testSuccessful = dev->Open();

        if (false != testSuccessful)
        {
            std::cout << "Open Succeeded: " << dev->GetName() << std::endl;
        }
        else
        {
            std::cout << "Open Failed: " << dev->GetName() << std::endl;
            break;
        }
    }

    return testSuccessful;
}

bool
UBlockDeviceTest::TestClose(void)
{
    bool testSuccessful = false;

    for (UBlockDevice* dev : deviceList)
    {
        testSuccessful = dev->Close();
        if (false != testSuccessful)
        {
            std::cout << "Close Succeeded: " << dev->GetName() << std::endl;
        }
        else
        {
            std::cout << "Close Failed: " << dev->GetName() << std::endl;
            break;
        }
    }

    return testSuccessful;
}

bool
UBlockDeviceTest::TestAsyncIO(void)
{
    bool testSuccessful = false;

    do
    {
        overallTestSuccessful = true;
        openDone = false;
        closeDone = false;
        threadList.clear();

        for (uint32_t threadIndex = 0; threadIndex < threadCount; threadIndex++)
        {
            registerThreadDone = false;
            threadTestDoneList[threadIndex] = false;

            std::thread* testThread = new std::thread(&_ASyncIOTestThread, this, threadIndex);
            threadList.push_back(testThread);

            WaitThreadRegisterDone();
        }

        testSuccessful = TestOpen();
        if (false == testSuccessful)
        {
            break;
        }

        std::cout << "Test Normal SubmitAsyncIO: " << std::endl;
        openDone = testSuccessful;

        for (uint32_t threadIndex = 0; threadIndex < threadCount; threadIndex++)
        {
            _WaitThreadTestDone(threadIndex);
        }

        testSuccessful = TestClose();
        if (false == testSuccessful)
        {
            break;
        }

        closeDone = testSuccessful;

        for (std::thread* testThread : threadList)
        {
            testThread->join();
        }

        testSuccessful = overallTestSuccessful;
    } while (false);

    for (std::thread* testThread : threadList)
    {
        delete testThread;
    }

    if (false != testSuccessful)
    {
        std::cout << "SubmitAsyncIO Test Succeeded: " << std::endl;
    }
    else
    {
        std::cout << "SubmitAsyncIO Test Failed: " << std::endl;
    }

    return testSuccessful;
}

void
UBlockDeviceTest::_ASyncIOTestThread(UBlockDeviceTest* test, uint32_t threadIndex)
{
    test->RegisterThread();

    test->WaitOpenDone();

    {
        UbioSmartPtr ubioRead = test->_GenerateUbio();
        UbioSmartPtr ubioWrite = test->_GenerateUbio();
        ubioRead->dir = UbioDir::Read;
        ubioWrite->dir = UbioDir::Write;

        bool testSuccessful = test->_RequestAndVerifyAsyncIO(ubioRead, ubioWrite,
            threadIndex);
        if (false == testSuccessful)
        {
            test->SetTestFailed();
        }

        // ReleaseUbio is always successful.
        testSuccessful = test->_ReleaseUbio(ubioRead);
        testSuccessful = test->_ReleaseUbio(ubioWrite);
    }

    test->SetThreadTestDone(threadIndex);

    test->WaitCloseDone();

    test->UnregisterThread();
}

uint32_t
UBlockDeviceTest::_SubmitIO(UbioSmartPtr ioToSubmit, uint32_t threadIndex)
{
    uint32_t completionCount = static_cast<uint32_t>(
        ioToSubmit->GetUBlock()->SubmitAsyncIO(ioToSubmit));
    return completionCount;
}

uint32_t
UBlockDeviceTest::_CheckIOCompletion(UBlockDevice* dev, uint32_t threadIndex)
{
    uint32_t completionCount = static_cast<uint32_t>(dev->CompleteIOs());
    return completionCount;
}

bool
UBlockDeviceTest::_RequestAndVerifyAsyncIO(UbioSmartPtr ubioRead,
    UbioSmartPtr ubioWrite, uint32_t threadIndex)
{
    bool testSuccessful = false;

    for (UBlockDevice* dev : deviceList)
    {
        int completionTotal = 0;
        int completion = 0;

        _GeneratePattern(ubioWrite->GetBuffer(0), ubioWrite->GetSize());

        {
            PhysicalBlkAddr pba =
                {
                    .dev = new ArrayDevice(dev),
                    .lba = threadIndex* IO_PAGE_COUNT* Ubio::UNITS_PER_BLOCK};
            ubioRead->SetPba(pba);
            ubioWrite->SetPba(pba);
        }

        std::cout << "Async Write: " << std::endl;
        completion = _SubmitIO(ubioWrite, threadIndex);

        while (1 > completionTotal)
        {
            completion = _CheckIOCompletion(dev, threadIndex);
            completionTotal += completion;
        }

        std::cout << "Async Read: " << std::endl;
        completion = _SubmitIO(ubioRead, threadIndex);
        completionTotal += completion;

        while (2 > completionTotal)
        {
            completion = _CheckIOCompletion(dev, threadIndex);
            completionTotal += completion;
        }

        delete ubioRead->GetDev();
        delete ubioWrite->GetDev();

        testSuccessful = _VerifyPattern(ubioWrite->GetBuffer(0),
            ubioRead->GetBuffer(0),
            ubioWrite->GetSize());
        if (false == testSuccessful)
        {
            std::cout << "VerifyPattern Failed: " << std::endl;
            break;
        }
    }

    return testSuccessful;
}

UbioSmartPtr
UBlockDeviceTest::_GenerateUbio(void)
{
    UbioSmartPtr ubio(new Ubio(nullptr, IO_PAGE_COUNT * Ubio::UNITS_PER_BLOCK));
    return ubio;
}

bool
UBlockDeviceTest::_ReleaseUbio(UbioSmartPtr ubio)
{
    bool releaseSuccessful = true;

    return releaseSuccessful;
}

void
UBlockDeviceTest::_GeneratePattern(void* mem, unsigned int sizeInBytes)
{
    int* bucket = reinterpret_cast<int*>(mem);
    for (unsigned int index = 0; index < sizeInBytes / sizeof(int); index++)
    {
        bucket[index] = rand();
    }
}

bool
UBlockDeviceTest::_VerifyPattern(
    const void* dst, const void* src, unsigned int sizeInBytes)
{
    int memcmpResult = memcmp(dst, src, sizeInBytes);
    return (0 == memcmpResult);
}

void
UBlockDeviceTest::PrepareThreadRegister(void)
{
    registerThreadDone = false;
}

void
UBlockDeviceTest::RegisterThread(void)
{
    for (UBlockDevice* dev : deviceList)
    {
        dev->Close();
    }

    registerThreadDone = true;
}

void
UBlockDeviceTest::WaitThreadRegisterDone(void)
{
    while (false == registerThreadDone)
    {
        usleep(1);
    }
}

void
UBlockDeviceTest::PrepareThreadUnregister(void)
{
    unregisterThreadDone = false;
}

void
UBlockDeviceTest::UnregisterThread(void)
{
    unregisterLock.lock();

    for (UBlockDevice* dev : deviceList)
    {
        dev->Close();
    }

    unregisterThreadDone = true;
    unregisterLock.unlock();
}

void
UBlockDeviceTest::WaitThreadUnregisterDone(void)
{
    while (false == unregisterThreadDone)
    {
        usleep(1);
    }
}

void
UBlockDeviceTest::WaitOpenDone(void)
{
    while (false == openDone)
    {
        usleep(1);
    }
}

void
UBlockDeviceTest::WaitCloseDone(void)
{
    while (false == closeDone)
    {
        usleep(1);
    }
}

void
UBlockDeviceTest::SetTestFailed(void)
{
    overallTestSuccessful = false;
}

void
UBlockDeviceTest::SetThreadTestDone(uint32_t threadIndex)
{
    threadTestDoneList[threadIndex] = true;
}

void
UBlockDeviceTest::_WaitThreadTestDone(uint32_t threadIndex)
{
    while (false == threadTestDoneList[threadIndex])
    {
        usleep(1);
    }
}

void
UBlockDeviceTest::_PrintDevice(void)
{
    for (UBlockDevice* dev : deviceList)
    {
        std::cout << dev->GetName() << std::endl;
    }
}

void
UBlockDeviceTest::PrintUsage(void)
{
    std::cout << "Usage: sudo ublock_device_test DEVICE_NAME" << std::endl;
}

} // namespace ibofos
