#include <gtest/gtest.h>
#include <cstring>
#include "src/io/general_io/submit_async_byte_io.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/array/service/io_translator/i_io_translator_mock.h"
#include "test/unit-tests/io/general_io/submit_async_byte_io_mock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Matcher;

namespace pos {

class DummyCallbackByteIO : public Callback
{
public:
    explicit DummyCallbackByteIO(bool isFront)
    : Callback(isFront)
    {
        callbackExecuted = false;
    }
    ~DummyCallbackByteIO(void) override{};
    bool callbackExecuted;

private:
    bool
    _DoSpecificJob(void)
    {
        callbackExecuted = true;
        return true;
    }
};

TEST(AsyncByteIO, AsyncByteIO_Constructor)
{
    AsyncByteIO asyncByteIO;
    MockIIOTranslator mockIIOTranslator;
    AsyncByteIO asyncByteIO2(&mockIIOTranslator);
}

TEST(AsyncByteIO, Byte_Execute)
{
    MockIIOTranslator mockIIOTranslator;
    MockIIOTranslator mockIIOTranslatorSuccess;
    // Given: io translator is created.
    // When : asyncByteIO is created.
    LogicalByteAddr startLSA;
    startLSA.byteOffset = 0;
    startLSA.byteSize = 8;
    PartitionType partitionToIO = PartitionType::META_NVM;
    AsyncByteIO asyncByteIO(&mockIIOTranslator);
    DummyCallbackByteIO* dummyCallbackByteIO = new DummyCallbackByteIO(true);
    CallbackSmartPtr callback(dummyCallbackByteIO);

    // Then : Execute but buffer is nullptr
    IOSubmitHandlerStatus ioSubmitHandlerStatus =
        asyncByteIO.Execute(IODirection::READ, nullptr, startLSA,
        partitionToIO, callback, 0);
    EXPECT_EQ(ioSubmitHandlerStatus, IOSubmitHandlerStatus::FAIL);
    EXPECT_EQ(dummyCallbackByteIO->callbackExecuted, false);

    // When : asyncByteIO is created, and buffer is valid
    PartitionType partitionToIOFail = PartitionType::META_SSD;
    void* buf = malloc(8);
    // Then : Execute but partition type is ssd
    // (if ssd also needs to use byte access, please change test)
    ioSubmitHandlerStatus =
        asyncByteIO.Execute(IODirection::READ, buf, startLSA,
        partitionToIOFail, callback, 0);
    EXPECT_EQ(ioSubmitHandlerStatus, IOSubmitHandlerStatus::FAIL);
    EXPECT_EQ(dummyCallbackByteIO->callbackExecuted, false);

    // When : asyncByteIO is created, and buffer is valid
    // Then : Execute but direction is not valid (neither read / write)
    ioSubmitHandlerStatus =
        asyncByteIO.Execute(IODirection::TRIM, buf, startLSA,
        partitionToIO, callback, 0);
    EXPECT_EQ(ioSubmitHandlerStatus, IOSubmitHandlerStatus::FAIL);
    EXPECT_EQ(dummyCallbackByteIO->callbackExecuted, false);

    // When : asyncByteIO is created. Buffer is valid.
    // Then : translator return fail status
    unsigned int arrayIdx = 0;
    EXPECT_CALL(mockIIOTranslator, ByteTranslate(Matcher<unsigned int>(arrayIdx), _, _, _)).WillRepeatedly(Return(1));
    ioSubmitHandlerStatus =
        asyncByteIO.Execute(IODirection::READ, buf, startLSA,
        partitionToIO, callback, 0);

    EXPECT_EQ(ioSubmitHandlerStatus, IOSubmitHandlerStatus::FAIL);
    EXPECT_EQ(dummyCallbackByteIO->callbackExecuted, false);

    // When : asyncByteIO is created. Buffer is valid.
    // Then : translator(covert, write case) return fail status
    EXPECT_CALL(mockIIOTranslator, ByteConvert(Matcher<unsigned int>(arrayIdx), _, _, _)).WillRepeatedly(Return(1));
    ioSubmitHandlerStatus =
        asyncByteIO.Execute(IODirection::WRITE, buf, startLSA,
        partitionToIO, callback, 0);

    EXPECT_EQ(ioSubmitHandlerStatus, IOSubmitHandlerStatus::FAIL);
    EXPECT_EQ(dummyCallbackByteIO->callbackExecuted, false);
}


TEST(AsyncByteIO, GetAddresses_Success_Data_Compare)
{
    MockIIOTranslator mockIIOTranslator;

    // Given: io translator is created.
    //        given buffer is set as zero and arrayBuffer is set as simple string

    PartitionType partitionToIO = PartitionType::META_NVM;
    MockAsyncByteIO asyncByteIO(&mockIIOTranslator);

    const char originSentence[] = "GetReadAddress_Success_Data_Compare";
    uint32_t size = sizeof(originSentence);
    char* buf = static_cast<char*>(malloc(size));
    char* arrayBuf = static_cast<char*>(malloc(size));

    LogicalByteAddr startLSA;
    startLSA.byteOffset = 0;
    startLSA.byteSize = size;

    memset(buf, 0, size);
    memcpy(arrayBuf, originSentence, size);

    // When : Read operation
    DummyCallbackByteIO* dummyCallbackByteIO = new DummyCallbackByteIO(true);
    CallbackSmartPtr callback(dummyCallbackByteIO);
    EXPECT_CALL(asyncByteIO, _GetReadAddress).WillRepeatedly(Return(static_cast<void *>(arrayBuf)));

    // Then: Execute and Compare two buffers
    IOSubmitHandlerStatus ioSubmitHandlerStatus =
        asyncByteIO.Execute(IODirection::READ, buf, startLSA,
        partitionToIO, callback, 0);
    int ret = strncmp(buf, arrayBuf, size);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(ioSubmitHandlerStatus, IOSubmitHandlerStatus::SUCCESS);
    EXPECT_EQ(dummyCallbackByteIO->callbackExecuted, true);

    // Given: io translator is created.
    //        given buffer is set as simple string and arrayBuffer is set as zero

    memset(arrayBuf, 0, size);
    memcpy(buf, originSentence, size);
    dummyCallbackByteIO->callbackExecuted = false;

    // When : Write operation
    EXPECT_CALL(asyncByteIO, _GetWriteAddress).WillRepeatedly(Return(static_cast<void *>(arrayBuf)));

    // Then: Execute and Compare two buffers
    ioSubmitHandlerStatus =
        asyncByteIO.Execute(IODirection::READ, buf, startLSA,
        partitionToIO, callback, 0);
    ret = strncmp(buf, arrayBuf, size);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(ioSubmitHandlerStatus, IOSubmitHandlerStatus::SUCCESS);
    EXPECT_EQ(dummyCallbackByteIO->callbackExecuted, true);
    free(buf);
    free(arrayBuf);
}

}  // namespace pos
