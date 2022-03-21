#include "src/io/frontend_io/write_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/array_mgmt/interface/i_array_mgmt_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/io/general_io/rba_state_manager_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/mapper/i_reversemap_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(WriteCompletion, WriteCompletion_OneArgument_Stack)
{
    //Given
    int arrayId = 0;
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(new NiceMock<MockVolumeIo>(nullptr, 1, 0));

    //When: Create new WriteCompletion with single argument
    WriteCompletion writeCompletion(volIo);

    //Then: Do nothing
}

TEST(WriteCompletion, WriteCompletion_OneArgument_Heap)
{
    //Given
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(new NiceMock<MockVolumeIo>(nullptr, 1, 0));

    //When: Create new WriteCompletion with single argument
    WriteCompletion* writeCompletion = new WriteCompletion(volIo);

    delete writeCompletion;

    //Then: Do nothing
}

TEST(WriteCompletion, WriteCompletion_FiveArgument_Stack)
{
    //Given
    NiceMock<MockIArrayMgmt> mockIArrayMgmt;
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(new NiceMock<MockVolumeIo>(nullptr, 1, 0));
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;

    //When: Create new WriteCompletion with 2 arguments
    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator, false, nullptr);

    //Then: Do nothing
}

TEST(WriteCompletion, WriteCompletion_FiveArgument_Heap)
{
    //Given
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(new NiceMock<MockVolumeIo>(nullptr, 1, 0));
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;

    //When: Create new WriteCompletion with 2 arguments
    WriteCompletion* writeCompletion = new WriteCompletion(volIo, &mockIWBStripeAllocator, false, nullptr);

    delete writeCompletion;

    //Then: Do nothing
}

TEST(WriteCompletion, _DoSpecificJob_NullStripe)
{
    //Given
    bool actual;
    NiceMock<MockIArrayMgmt> mockIArrayMgmt;
    auto mockVolIo = new NiceMock<MockVolumeIo>(nullptr, 1, 0);
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(mockVolIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager("", 0);
    StripeAddr stripeAddr;
    Stripe stripe(nullptr, true, 1);
    VirtualBlkAddr startVsa;

    ON_CALL(*mockVolIo, GetLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolIo, GetVsa()).WillByDefault(ReturnRef(startVsa));

    //When: Execute WriteCompletion with null stripe
    ON_CALL(mockIWBStripeAllocator, GetStripe).WillByDefault(Return(nullptr));

    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator, false, &mockIArrayMgmt);
    actual = writeCompletion.Execute();

    //Then: WriteCompletion should return success
    ASSERT_EQ(actual, true);
}

TEST(WriteCompletion, _RequestFlush_DummyStripe)
{
    //Given
    bool actual;
    NiceMock<MockIArrayInfo> mockIArrayInfo;
    NiceMock<MockIArrayMgmt> mockIArrayMgmt;
    auto mockVolIo = new NiceMock<MockVolumeIo>(nullptr, 1, 0);
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(mockVolIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager("", 0);
    StripeAddr stripeAddr;
    NiceMock<MockIReverseMap> rev;
    Stripe stripe(&rev, true, 1);
    VirtualBlkAddr startVsa;
    int arrayId = 0;
    ComponentsInfo info{&mockIArrayInfo, nullptr};
    ON_CALL(rev, Flush).WillByDefault(Return(0));
    ON_CALL(*mockVolIo, GetLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolIo, GetVsa()).WillByDefault(ReturnRef(startVsa));
    ON_CALL(mockIArrayMgmt, GetInfo(arrayId)).WillByDefault(Return(&info));
    ON_CALL(mockIArrayInfo, IsWriteThroughEnabled()).WillByDefault(Return(false));

    //When: Execute WriteCompletion with dummy stripe
    ON_CALL(mockIWBStripeAllocator, GetStripe).WillByDefault(Return(&stripe));

    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator, false, &mockIArrayMgmt);
    actual = writeCompletion.Execute();

    //Then: WriteCompletion should return success
    ASSERT_EQ(actual, true);
}

TEST(WriteCompletion, _ReqeustFlush_FlushSuccess)
{
    //Given
    bool actual;
    NiceMock<MockIArrayInfo> mockIArrayInfo;
    NiceMock<MockIArrayMgmt> mockIArrayMgmt;
    auto mockVolIo = new NiceMock<MockVolumeIo>(nullptr, 1, 0);
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(mockVolIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager("", 0);
    NiceMock<MockStripe> mockStripe;
    StripeAddr stripeAddr;
    VirtualBlkAddr startVsa;
    int arrayId = 0;
    ComponentsInfo info{&mockIArrayInfo, nullptr};

    ON_CALL(*mockVolIo, GetLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolIo, GetVsa()).WillByDefault(ReturnRef(startVsa));

    //When: Execute WriteCompletion causing flush returning success
    ON_CALL(mockStripe, DecreseBlksRemaining(_)).WillByDefault(Return(0));
    ON_CALL(mockStripe, Flush(_)).WillByDefault(Return(0));
    ON_CALL(mockIWBStripeAllocator, GetStripe).WillByDefault(Return(&mockStripe));
    ON_CALL(mockIArrayMgmt, GetInfo(arrayId)).WillByDefault(Return(&info));
    ON_CALL(mockIArrayInfo, IsWriteThroughEnabled()).WillByDefault(Return(false));
    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator, false, &mockIArrayMgmt);
    actual = writeCompletion.Execute();

    //Then: WriteCompletion should return success
    ASSERT_EQ(actual, true);
}

TEST(WriteCompletion, _RequestFlush_FlushError)
{
    //Given
    bool actual;
    NiceMock<MockIArrayInfo> mockIArrayInfo;
    NiceMock<MockIArrayMgmt> mockIArrayMgmt;
    auto mockVolIo = new NiceMock<MockVolumeIo>(nullptr, 1, 0);
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(mockVolIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager("", 0);
    NiceMock<MockStripe> mockStripe;
    StripeAddr stripeAddr;
    Stripe stripe(nullptr, true, 1);
    VirtualBlkAddr startVsa;
    StripeId stripeId = 1;
    int arrayId = 0;
    ComponentsInfo info{&mockIArrayInfo, nullptr};

    ON_CALL(*mockVolIo, GetLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolIo, GetVsa()).WillByDefault(ReturnRef(startVsa));

    //When: Execute WriteCompletion causing flush returning failure
    ON_CALL(mockStripe, DecreseBlksRemaining(_)).WillByDefault(Return(0));
    ON_CALL(mockStripe, Flush(_)).WillByDefault(Return(-1));
    ON_CALL(mockIWBStripeAllocator, GetStripe).WillByDefault(Return(&mockStripe));
    ON_CALL(mockIArrayMgmt, GetInfo(arrayId)).WillByDefault(Return(&info));
    ON_CALL(mockIArrayInfo, IsWriteThroughEnabled()).WillByDefault(Return(false));
    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator, false, &mockIArrayMgmt);
    actual = writeCompletion.Execute();

    //Then: WriteCompletion should return success
    ASSERT_EQ(actual, true);
}

} // namespace pos
