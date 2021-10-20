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

#include "src/io_scheduler/io_worker_device_operation.h"

#include <gtest/gtest.h>

#include <thread>
#include <unistd.h>

#include "src/device/base/ublock_device.h"

namespace pos
{
class StubUBlockDeviceIWDO : public UBlockDevice
{
public:
    StubUBlockDeviceIWDO(void)
    : UBlockDevice("", 0, nullptr)
    {
    }

protected:
    virtual DeviceContext* _AllocateDeviceContext(void) final
    {
        return nullptr;
    }
    virtual void _ReleaseDeviceContext(DeviceContext* deviceContextToRelease) final
    {
    }
};

TEST(IoWorkerDeviceOperation, IoWorkerDeviceOperation_Stack)
{
    // Given: Do nothing

    // When: Create IoWorkerDeviceOperation
    IoWorkerDeviceOperation ioWorkerDeviceOperation {IoWorkerDeviceOperationType::INSERT, nullptr};

    // Then: Do nothing
}

TEST(IoWorkerDeviceOperation, IoWorkerDeviceOperation_Heap)
{
    // Given: Do nothing

    // When: Create IoWorkerDeviceOperation
    IoWorkerDeviceOperation* ioWorkerDeviceOperation = new IoWorkerDeviceOperation {IoWorkerDeviceOperationType::INSERT, nullptr};
    delete ioWorkerDeviceOperation;

    // Then: Do nothing
}

TEST(IoWorkerDeviceOperation, WaitDone_SimpleCall)
{
    // Given: IoWorkerDeviceOperation
    IoWorkerDeviceOperation ioWorkerDeviceOperation {IoWorkerDeviceOperationType::INSERT, nullptr};
    std::thread t1([&](void) -> void
    {
        usleep(10000); // 10ms
        ioWorkerDeviceOperation.SetDone();
    });

    // When: Call WaitDone
    ioWorkerDeviceOperation.WaitDone();
    t1.join();

    // Then: Do nothing
}

TEST(IoWorkerDeviceOperation, SetDone_SimpleCall)
{
    // Given: IoWorkerDeviceOperation
    IoWorkerDeviceOperation ioWorkerDeviceOperation {IoWorkerDeviceOperationType::INSERT, nullptr};

    // When: Call SetDone
    ioWorkerDeviceOperation.SetDone();

    // Then: Do nothing
}

TEST(IoWorkerDeviceOperation, GetDevice_SimpleCall)
{
    // Given: IoWorkerDeviceOperation, StubUBlockDeviceIWDO
    auto ublock = std::make_shared<StubUBlockDeviceIWDO>();
    IoWorkerDeviceOperation ioWorkerDeviceOperation {IoWorkerDeviceOperationType::INSERT, ublock};
    UblockSharedPtr actual;

    // When: Call GetDevice
    actual = ioWorkerDeviceOperation.GetDevice();

    // Then: Expect to return ublock
    EXPECT_EQ(actual.get(), ublock.get());
}

TEST(IoWorkerDeviceOperation, GetCommand_SimpleCall)
{
    // Given: IoWorkerDeviceOperation
    IoWorkerDeviceOperation ioWorkerDeviceOperation {IoWorkerDeviceOperationType::REMOVE, nullptr};
    IoWorkerDeviceOperationType actual, expected = IoWorkerDeviceOperationType::REMOVE;

    // When: Call GetCommand
    actual = ioWorkerDeviceOperation.GetCommand();

    // Then: Expect to return IoWorkerDeviceOperationType::REMOVE
    EXPECT_EQ(actual, expected);
}

} // namespace pos
