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

#include "src/event_scheduler/io_completer.h"

#include <gtest/gtest.h>

#include "src/event_scheduler/event_factory.h"
#include "src/include/core_const.h"

#include "test/unit-tests/bio/ubio_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::AtLeast;
using ::testing::Return;

namespace pos
{

class StubCallbackIOC : public Callback
{
public:
    StubCallbackIOC(void)
    : Callback(false)
    {
    }

private:
    virtual bool _DoSpecificJob(void) final
    {
        return true;
    }
};

class StubEventFactoryIOC : public EventFactory
{
public:
    virtual EventSmartPtr Create(UbioSmartPtr ubio) final
    {
        return nullptr;
    }
};

TEST(IoCompleter, IoCompleter_Stack)
{
    // Given: Do nothing

    // When: Create IoCompleter
    IoCompleter ioCompleter {nullptr};

    // Then: Do nothing
}

TEST(IoCompleter, IoCompleter_Heap)
{
    // Given: Do nothing

    // When: Create IoCompleter
    IoCompleter* ioCompleter = new IoCompleter {nullptr};
    delete ioCompleter;

    // Then: Do nothing
}

TEST(IoCompleter, CompleteOriginUbio_OriginUbioNull)
{
    // Given: MockUbio, IoCompleter
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), GetOriginUbio()).WillByDefault(Return(nullptr));
    EXPECT_CALL(*ubio.get(), GetOriginUbio()).Times(AtLeast(1));
    IoCompleter ioCompleter {ubio};

    // When: Call CompleteOriginUbio
    ioCompleter.CompleteOriginUbio();

    // Then: Do nothing
}

TEST(IoCompleter, CompleteOriginUbio_OriginUbio_CallbackNull)
{
    // Given: MockUbio, IoCompleter
    auto origin = std::make_shared<MockUbio>(nullptr, 0, 0);
    EXPECT_CALL(*origin.get(), GetCallback()).Times(AtLeast(1));
    EXPECT_CALL(*origin.get(), ClearCallback()).Times(AtLeast(1));
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), GetOriginUbio()).WillByDefault(Return(origin));
    ON_CALL(*ubio.get(), GetCallback()).WillByDefault(Return(nullptr));
    EXPECT_CALL(*ubio.get(), GetOriginUbio()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), GetCallback()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), ClearOrigin()).Times(AtLeast(1));
    IoCompleter ioCompleter {ubio};

    // When: Call CompleteOriginUbio
    ioCompleter.CompleteOriginUbio();

    // Then: Do nothing
}

TEST(IoCompleter, CompleteOriginUbio_OriginUbio_Callback)
{
    // Given: StubCallbackIOC, MockUbio, IoCompleter
    auto callback = std::make_shared<StubCallbackIOC>();
    auto origin = std::make_shared<MockUbio>(nullptr, 0, 0);
    EXPECT_CALL(*origin.get(), GetCallback()).Times(AtLeast(1));
    EXPECT_CALL(*origin.get(), ClearCallback()).Times(AtLeast(1));
    EXPECT_CALL(*origin.get(), Complete(_)).Times(AtLeast(1));
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), GetOriginUbio()).WillByDefault(Return(origin));
    ON_CALL(*ubio.get(), GetCallback()).WillByDefault(Return(callback));
    EXPECT_CALL(*ubio.get(), GetOriginUbio()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), GetCallback()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), ClearOrigin()).Times(AtLeast(1));
    IoCompleter ioCompleter {ubio};

    // When: Call CompleteOriginUbio
    ioCompleter.CompleteOriginUbio();

    // Then: Do nothing
}

TEST(IoCompleter, CompleteUbio_SuccessType)
{
    // Given: MockUbio, IoCompleter
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), Complete(_)).WillByDefault(Return());
    ON_CALL(*ubio.get(), IsSyncMode()).WillByDefault(Return(false));
    EXPECT_CALL(*ubio.get(), Complete(_)).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), IsSyncMode()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), CheckRecoveryAllowed()).Times(AtLeast(1));
    IoCompleter ioCompleter {ubio};

    // When 1: Call CompleteUbio, Do not allow recovery
    ON_CALL(*ubio.get(), CheckRecoveryAllowed()).WillByDefault(Return(false));
    ioCompleter.CompleteUbio(IOErrorType::SUCCESS, false);

    // When 2: Call CompleteUbio, Allow recovery
    ON_CALL(*ubio.get(), CheckRecoveryAllowed()).WillByDefault(Return(true));
    ioCompleter.CompleteUbio(IOErrorType::SUCCESS, false);

    // Then: Do nothing
}

TEST(IoCompleter, CompleteUbio_ErrorType)
{
    // Given: MockUbio, MockEventScheduler, IoCompleter
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), Complete(_)).WillByDefault(Return());
    ON_CALL(*ubio.get(), IsSyncMode()).WillByDefault(Return(false));
    EXPECT_CALL(*ubio.get(), Complete(_)).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), IsSyncMode()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), CheckRecoveryAllowed()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), SetError(_)).Times(AtLeast(1));
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    IoCompleter ioCompleter {ubio, &mockEventScheduler};

    // When 1: Call CompleteUbio, Do not allow recovery
    ON_CALL(*ubio.get(), CheckRecoveryAllowed()).WillByDefault(Return(false));
    ioCompleter.CompleteUbio(IOErrorType::GENERIC_ERROR, false);

    // When 2: Call CompleteUbio, Allow recovery
    ON_CALL(*ubio.get(), CheckRecoveryAllowed()).WillByDefault(Return(true));
    ON_CALL(*ubio.get(), SetError(_)).WillByDefault(Return());
    EXPECT_ANY_THROW(ioCompleter.CompleteUbio(IOErrorType::GENERIC_ERROR, false));

    // When 3: Call CompleteUbio, Allow recovery with StubEventFactoryIOC
    StubEventFactoryIOC stubEventFactoryIOC;
    ioCompleter.RegisterRecoveryEventFactory(&stubEventFactoryIOC);
    ioCompleter.CompleteUbio(IOErrorType::GENERIC_ERROR, false);

    // Then: Do nothing
}

TEST(IoCompleter, CompleteUbioWithoutRecovery_NotExecute)
{
    // Given: MockUbio, IoCompleter
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), Complete(_)).WillByDefault(Return());
    ON_CALL(*ubio.get(), IsSyncMode()).WillByDefault(Return(true));
    EXPECT_CALL(*ubio.get(), Complete(_)).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), IsSyncMode()).Times(AtLeast(1));
    IoCompleter ioCompleter {ubio};

    // When 1: Call CompleteUbioWithoutRecovery with SyncMode
    ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::SUCCESS, false);

    // When 2: Call CompleteUbioWithoutRecovery with AsyncMode
    ON_CALL(*ubio.get(), IsSyncMode()).WillByDefault(Return(false));
    ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::SUCCESS, false);

    // Then: Do nothing
}

TEST(IoCompleter, CompleteUbioWithoutRecovery_Execute)
{
    // Given: StubCallbackIOC, MockUbio, MockEventScheduler, MockEventFrameworkApi, IoCompleter
    auto callback = std::make_shared<StubCallbackIOC>();
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), Complete(_)).WillByDefault(Return());
    ON_CALL(*ubio.get(), GetCallback()).WillByDefault(Return(callback));
    ON_CALL(*ubio.get(), ClearCallback()).WillByDefault(Return());
    EXPECT_CALL(*ubio.get(), Complete(_)).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), IsSyncMode()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), GetCallback()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), ClearCallback()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), GetOriginCore()).Times(AtLeast(1));
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    IoCompleter ioCompleter {ubio, &mockEventScheduler, &mockEventFrameworkApi};

    // When 1: Call CompleteUbioWithoutRecovery with SyncMode
    ON_CALL(*ubio.get(), IsSyncMode()).WillByDefault(Return(true));
    ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::SUCCESS, true);

    // When 2: Call CompleteUbioWithoutRecovery with AsyncMode & INVALID_CORE
    ON_CALL(*ubio.get(), IsSyncMode()).WillByDefault(Return(false));
    ON_CALL(*ubio.get(), GetOriginCore()).WillByDefault(Return(INVALID_CORE));
    ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::SUCCESS, true);

    // When 3: Call CompleteUbioWithoutRecovery with AsyncMode & SameReactor
    ON_CALL(*ubio.get(), GetOriginCore()).WillByDefault(Return(1));
    ON_CALL(mockEventFrameworkApi, IsSameReactorNow(_)).WillByDefault(Return(true));
    ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::SUCCESS, true);

    // When 4: Call CompleteUbioWithoutRecovery with AsyncMode & NotSameReactor
    ON_CALL(mockEventFrameworkApi, IsSameReactorNow(_)).WillByDefault(Return(false));
    ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::SUCCESS, true);

    // Then: Do nothing
}

TEST(IoCompleter, RegisterRecoveryEventFactory_SimpleCall)
{
    // Given: StubEventFactoryIOC, IoCompleter
    StubEventFactoryIOC stubEventFactoryIOC;
    IoCompleter ioCompleter {nullptr};

    // When: Call RegisterRecoveryEventFactory
    ioCompleter.RegisterRecoveryEventFactory(&stubEventFactoryIOC);

    // Then: Do nothing
}

} // namespace pos
