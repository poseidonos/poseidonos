#include "src/io/frontend_io/read_completion_for_partial_write.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/io/general_io/translator_mock.h"
#include "src/include/address_type.h"
#include "src/include/smart_ptr_type.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{

TEST(ReadCompletionForPartialWrite, ReadCompletionForPartialWrite_Constructor)
{
    // Given
    const uint32_t unitCount = 8;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>((void *)0xff00, unitCount, 0));

    // When : Test contstructor is properly done.
    ReadCompletionForPartialWrite readCompletionForPartialWrite(volumeIo, 4096, 0, &mockIWBStripeAllocator, true);
}

TEST(ReadCompletionForPartialWrite, ReadCompletionForPartialWrite_DoSpecificJob)
{
    // Given : Normal case.
    const uint32_t unitCount = 8;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    MockVolumeIo *mockVolumeIo = new NiceMock<MockVolumeIo>((void *)0xff00, unitCount, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    MockVolumeIo *mockVolumeIoOrigin = new NiceMock<MockVolumeIo>((void *)0xff00, unitCount, 0);
    VolumeIoSmartPtr volumeIoOrigin(mockVolumeIoOrigin);
    StripeAddr stripeAddr = { .stripeLoc = IN_USER_AREA, .stripeId = 0};
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};

    ON_CALL(*mockVolumeIo, GetOldLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolumeIo, GetOriginVolumeIo()).WillByDefault(Return(volumeIoOrigin));
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    EXPECT_CALL(mockIWBStripeAllocator, DereferLsidCnt);

    // When : Execute is properly done and check DereferLsidCnt is called.
    ReadCompletionForPartialWrite readCompletionForPartialWrite(volumeIo, 4096, 0, &mockIWBStripeAllocator, true);
    bool expected = true;
    bool actual = readCompletionForPartialWrite.Execute();
    ASSERT_EQ(actual, expected);
    volumeIo = nullptr;
    mockVolumeIoOrigin = nullptr;
}

TEST(ReadCompletionForPartialWrite, ReadCompletionForPartialWrite_DoSpecificJob_SplitNull)
{
    // Given : If origin of volume IO is nullptr
    const uint32_t unitCount = 8;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    MockVolumeIo *mockVolumeIo = new NiceMock<MockVolumeIo>((void *)0xff00, unitCount, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    StripeAddr stripeAddr = { .stripeLoc = IN_USER_AREA, .stripeId = 0};
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};

    ON_CALL(*mockVolumeIo, GetOldLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolumeIo, GetOriginVolumeIo()).WillByDefault(Return(nullptr));
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));

    // When : readCompletionForPartialWrite is executed without seg fault error
    ReadCompletionForPartialWrite readCompletionForPartialWrite(volumeIo, 4096, 0, &mockIWBStripeAllocator, true);
    bool expected = true;
    bool actual = readCompletionForPartialWrite.Execute();
    volumeIo = nullptr;
    ASSERT_EQ(actual, expected);
}

TEST(ReadCompletionForPartialWrite, ReadCompletionForPartialWrite_HandleCopyDone)
{
    // Given : Buffer Entry / Old
    const uint32_t unitCount = 8;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    MockVolumeIo *mockVolumeIo = new NiceMock<MockVolumeIo>((void *)0xff00, unitCount, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    MockVolumeIo *mockVolumeIoOrigin = new NiceMock<MockVolumeIo>((void *)0xff00, unitCount, 0);
    VolumeIoSmartPtr volumeIoOrigin(mockVolumeIoOrigin);
    StripeAddr stripeAddr = {.stripeLoc = IN_USER_AREA, .stripeId = 0};
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    MockTranslator* translator = new NiceMock<MockTranslator>(vsa, volumeIo->GetArrayId(), iArrayInfo, UNMAP_STRIPE);
    CopyParameter* copyParameter = new CopyParameter(volumeIo, translator, true);
    BufferEntry bufferEntry((void *)0xff00, 1);
    PhysicalEntry physicalEntry;

    list<PhysicalEntry> physicalEntries;
    physicalEntries.push_back(physicalEntry);
    void *argument = (static_cast<void*>(copyParameter));

    ON_CALL(*mockVolumeIo, GetOldLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolumeIo, GetOriginVolumeIo()).WillByDefault(Return(volumeIoOrigin));
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*translator, GetPhysicalEntries(_, _)).WillByDefault(Return(physicalEntries));

    // Handle Done is executeded without any error with given condition
    ReadCompletionForPartialWrite::HandleCopyDone(argument);
    delete translator;
}

} // namespace pos
