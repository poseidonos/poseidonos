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


#include "tool/library_unit_test/library_unit_test.h"
ibofos::LibraryUnitTest libraryUnitTest;

#include <cstdio>
#include <iostream>

#include "mk/ibof_config.h"
#include "src/io/general_io/error_type.h"
#include "src/io/general_io/ubio.h"
#include "src/main/ibofos.h"
#include "src/scheduler/callback.h"

namespace ibofos
{
volatile int testCount = 0;
CallbackError retError;
volatile int errCount = 0;

class DummyCallbackHandler : public Callback
{
public:
    DummyCallbackHandler(bool isFront, UbioSmartPtr inputUbio = nullptr)
    : Callback(isFront),
      ubio(inputUbio)
    {
    }
    ~DummyCallbackHandler() override{};

private:
    bool completeOrigin;
    UbioSmartPtr ubio;
    bool
    _DoSpecificJob()
    {
        errCount = _GetErrorCount();
        retError = _GetMostCriticalError();
        if (ubio)
        {
            ubio->CompleteOrigin();
        }
        testCount++;
        return true;
    }
};

// Test 1 Indicates

void
clean_test()
{
    testCount = 0;
    errCount = 0;
}

void
test1_success()
{
    clean_test();

    libraryUnitTest.TestStart(1);

    CallbackSmartPtr callback1(new DummyCallbackHandler(true));
    CallbackSmartPtr callback2(new DummyCallbackHandler(false));
    callback1->SetCallee(callback2);
    callback1->InformError(CallbackError::SUCCESS);
    callback1->Execute();
    while (testCount < 2)
    {
        usleep(1);
    }

    libraryUnitTest.TestResult(1, (retError == CallbackError::CALLBACK_ERROR_MAX_COUNT) && errCount == 0);
}

void
test2_multiple_errors_1()
{
    clean_test();

    libraryUnitTest.TestStart(2);

    CallbackSmartPtr callback1(new DummyCallbackHandler(true));
    CallbackSmartPtr callback2(new DummyCallbackHandler(false));
    callback1->SetCallee(callback2);
    callback1->InformError(CallbackError::ABORTED);
    callback2->InformError(CallbackError::TRANSPORT_FAIL);
    callback1->Execute();
    while (testCount < 2)
    {
        usleep(1);
    }

    libraryUnitTest.TestResult(2, (retError == CallbackError::TRANSPORT_FAIL) && errCount == 2);
}

void
test3_multiple_errors_2()
{
    clean_test();

    libraryUnitTest.TestStart(3);

    CallbackSmartPtr callback1(new DummyCallbackHandler(true));
    CallbackSmartPtr callback2(new DummyCallbackHandler(false));
    callback1->SetCallee(callback2);
    callback2->InformError(CallbackError::ABORTED);
    callback1->InformError(CallbackError::SUCCESS);
    callback1->Execute();
    while (testCount < 2)
    {
        usleep(1);
    }

    libraryUnitTest.TestResult(3, (retError == CallbackError::ABORTED) && errCount == 1);
}

void
test4_multiple_errors_3()
{
    clean_test();
    libraryUnitTest.TestStart(4);
    CallbackSmartPtr callback1(new DummyCallbackHandler(true));
    CallbackSmartPtr callback2(new DummyCallbackHandler(false));
    CallbackSmartPtr callback3(new DummyCallbackHandler(false));
    callback1->SetCallee(callback2);
    callback3->SetCallee(callback2);
    callback2->SetWaitingCount(2);

    callback3->InformError(CallbackError::ABORTED);
    callback1->InformError(CallbackError::REFERENCE_INCREASE_FAIL);
    callback2->InformError(CallbackError::TRANSPORT_FAIL);
    callback1->Execute();
    callback3->Execute();
    while (testCount < 3)
    {
        usleep(1);
    }

    libraryUnitTest.TestResult(4, (retError == CallbackError::TRANSPORT_FAIL) && errCount == 3);
}

void
test5_ubio_simple_1()
{
    clean_test();
    libraryUnitTest.TestStart(5);
    void* mem = ibofos::Memory<512>::Alloc(8);
    CallbackSmartPtr callback1(new DummyCallbackHandler(true));
    UbioSmartPtr ubio(new Ubio(mem, 8));
    ubio->SetCallback(callback1);
    ubio->CompleteWithoutRecovery(CallbackError::ABORTED);

    while (testCount < 1)
    {
        usleep(1);
    }
    ibofos::Memory<512>::Free(mem);
    libraryUnitTest.TestResult(5, (retError == CallbackError::ABORTED) && errCount == 1);
}

void
test6_ubio_simple_2()
{
    clean_test();

    libraryUnitTest.TestStart(6);
    void* mem = ibofos::Memory<512>::Alloc(8);
    CallbackSmartPtr callback1(new DummyCallbackHandler(true));
    CallbackSmartPtr callback2(new DummyCallbackHandler(true));
    UbioSmartPtr ubio(new Ubio(mem, 8));
    ubio->SetCallback(callback1);
    callback1->SetCallee(callback2);
    ubio->CompleteWithoutRecovery(CallbackError::GENERIC_ERROR);

    while (testCount < 2)
    {
        usleep(1);
    }
    ibofos::Memory<512>::Free(mem);
    libraryUnitTest.TestResult(6, (retError == CallbackError::GENERIC_ERROR) && errCount == 1);
}

void
test7_ubio_split_1()
{
    clean_test();

    libraryUnitTest.TestStart(7);
    void* mem1 = ibofos::Memory<512>::Alloc(8);
    void* mem2 = ibofos::Memory<512>::Alloc(8);
    UbioSmartPtr ubio(new Ubio(mem1, 8));
    UbioSmartPtr ubio2(new Ubio(mem2, 8));

    CallbackSmartPtr callback1(new DummyCallbackHandler(true));
    CallbackSmartPtr callback2(new DummyCallbackHandler(true));
    CallbackSmartPtr callback3(new DummyCallbackHandler(true));
    CallbackSmartPtr callback4(new DummyCallbackHandler(true, ubio));
    CallbackSmartPtr callback5(new DummyCallbackHandler(true));

    UbioSmartPtr split1 = ubio->Split(1, false);
    UbioSmartPtr split2 = ubio->Split(1, false);
    UbioSmartPtr split3 = ubio->Split(1, false);

    split1->SetCallback(callback1);
    split2->SetCallback(callback2);
    split3->SetCallback(callback3);
    callback4->SetWaitingCount(3);

    callback1->SetCallee(callback4);
    callback2->SetCallee(callback4);
    callback3->SetCallee(callback4);

    ubio2->SetCallback(callback5);
    ubio->SetCallback(callback4);
    ubio->SetOriginUbio(ubio2);
    split3->CompleteWithoutRecovery(CallbackError::GENERIC_ERROR);
    split2->CompleteWithoutRecovery(CallbackError::SUCCESS);
    split1->CompleteWithoutRecovery(CallbackError::ABORTED);

    while (testCount < 5)
    {
        usleep(1);
    }
    ibofos::Memory<512>::Free(mem1);
    ibofos::Memory<512>::Free(mem2);
    libraryUnitTest.TestResult(7, (retError == CallbackError::GENERIC_ERROR) && errCount == 2);
}

void
test8_ubio_split_2()
{
    clean_test();

    libraryUnitTest.TestStart(8);
    void* mem1 = ibofos::Memory<512>::Alloc(8);
    void* mem2 = ibofos::Memory<512>::Alloc(8);
    UbioSmartPtr ubio(new Ubio(mem1, 8));
    UbioSmartPtr ubio2(new Ubio(mem2, 8));

    CallbackSmartPtr callback1(new DummyCallbackHandler(true));
    CallbackSmartPtr callback2(new DummyCallbackHandler(true));
    CallbackSmartPtr callback3(new DummyCallbackHandler(true));
    CallbackSmartPtr callback4(new DummyCallbackHandler(true, ubio));
    CallbackSmartPtr callback5(new DummyCallbackHandler(true));

    UbioSmartPtr split1 = ubio->Split(1, false);
    UbioSmartPtr split2 = ubio->Split(1, false);
    UbioSmartPtr split3 = ubio->Split(1, false);

    split1->SetCallback(callback1);
    split2->SetCallback(callback2);
    split3->SetCallback(callback3);
    callback4->SetWaitingCount(3);

    callback1->SetCallee(callback4);
    callback2->SetCallee(callback4);
    callback3->SetCallee(callback4);

    ubio2->SetCallback(callback5);
    ubio->SetCallback(callback4);
    ubio->SetOriginUbio(ubio2);

    split3->CompleteWithoutRecovery(CallbackError::SUCCESS);
    split2->CompleteWithoutRecovery(CallbackError::SUCCESS);
    split1->CompleteWithoutRecovery(CallbackError::SUCCESS);

    while (testCount < 5)
    {
        usleep(1);
    }
    ibofos::Memory<512>::Free(mem1);
    ibofos::Memory<512>::Free(mem2);
    libraryUnitTest.TestResult(8, (retError == CallbackError::CALLBACK_ERROR_MAX_COUNT) && errCount == 0);
}

void
test9_tcmalloc_rte_malloc()
{
    libraryUnitTest.TestStart(9);
    void* mem0 = malloc(16);
    void* mem1 = ibofos::Memory<512>::Alloc(8);
    void* mem2 = malloc(16);
    void* mem3 = ibofos::Memory<512>::Alloc(8);
    ibofos::Memory<512>::Free(mem1);
    free(mem0);
    ibofos::Memory<512>::Free(mem3);
    free(mem2);
    libraryUnitTest.TestResult(9, (true));
}

} // namespace ibofos

int
main(int argc, char* argv[])
{
    libraryUnitTest.Initialize(argc, argv);

    ibofos::test1_success();
    ibofos::test2_multiple_errors_1();
    ibofos::test3_multiple_errors_2();
    ibofos::test4_multiple_errors_3();
    ibofos::test5_ubio_simple_1();
    ibofos::test6_ubio_simple_2();
    ibofos::test7_ubio_split_1();
    ibofos::test8_ubio_split_2();
    ibofos::test9_tcmalloc_rte_malloc();

    libraryUnitTest.SuccessAndExit();
    return 0;
}
