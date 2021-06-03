#include "src/io/frontend_io/write_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/wb_stripe_manager/stripe_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/io/general_io/rba_state_manager_mock.h"

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
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(new NiceMock<MockVolumeIo>(nullptr, 1, ""));

    //When: Create new WriteCompletion with single argument
    WriteCompletion writeCompletion(volIo);

    //Then: Do nothing
}

TEST(WriteCompletion, WriteCompletion_OneArgument_Heap)
{
    //Given
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(new NiceMock<MockVolumeIo>(nullptr, 1, ""));

    //When: Create new WriteCompletion with single argument
    WriteCompletion* writeCompletion = new WriteCompletion(volIo);

    delete writeCompletion;

    //Then: Do nothing
}

TEST(WriteCompletion, WriteCompletion_TwoArgument_Stack)
{
    //Given
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(new NiceMock<MockVolumeIo>(nullptr, 1, ""));
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;

    //When: Create new WriteCompletion with 2 arguments
    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator);

    //Then: Do nothing
}

TEST(WriteCompletion, WriteCompletion_TwoArgument_Heap)
{
    //Given
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(new NiceMock<MockVolumeIo>(nullptr, 1, ""));
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;

    //When: Create new WriteCompletion with 2 arguments
    WriteCompletion* writeCompletion = new WriteCompletion(volIo, &mockIWBStripeAllocator);

    delete writeCompletion;

    //Then: Do nothing
}

TEST(WriteCompletion, _DoSpecificJob_NullStripe)
{
    //Given
    bool actual;
    auto mockVolIo = new NiceMock<MockVolumeIo>(nullptr, 1, "");
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(mockVolIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager("");
    StripeAddr stripeAddr;
    Stripe stripe(nullptr, true);
    VirtualBlkAddr startVsa;

    ON_CALL(*mockVolIo, GetLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolIo, GetVsa()).WillByDefault(ReturnRef(startVsa));

    //When: Execute WriteCompletion with null stripe
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(nullptr));

    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator);
    actual = writeCompletion.Execute();

    //Then: WriteCompletion should return success
    ASSERT_EQ(actual, true);
}

TEST(WriteCompletion, _RequestFlush_DummyStripe)
{
    //Given
    bool actual;
    auto mockVolIo = new NiceMock<MockVolumeIo>(nullptr, 1, "");
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(mockVolIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager("");
    StripeAddr stripeAddr;
    Stripe stripe(nullptr, true);
    VirtualBlkAddr startVsa;

    ON_CALL(*mockVolIo, GetLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolIo, GetVsa()).WillByDefault(ReturnRef(startVsa));

    //When: Execute WriteCompletion with dummy stripe
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&stripe));

    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator);
    actual = writeCompletion.Execute();

    //Then: WriteCompletion should return success
    ASSERT_EQ(actual, true);
}

TEST(WriteCompletion, _ReqeustFlush_FlushSuccess)
{
    //Given
    bool actual;
    auto mockVolIo = new NiceMock<MockVolumeIo>(nullptr, 1, "");
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(mockVolIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager("");
    NiceMock<MockStripe> mockStripe;
    StripeAddr stripeAddr;
    VirtualBlkAddr startVsa;

    ON_CALL(*mockVolIo, GetLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolIo, GetVsa()).WillByDefault(ReturnRef(startVsa));

    //When: Execute WriteCompletion causing flush returning success
    ON_CALL(mockStripe, DecreseBlksRemaining(_)).WillByDefault(Return(0));
    ON_CALL(mockStripe, Flush(_)).WillByDefault(Return(0));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&mockStripe));
    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator);
    actual = writeCompletion.Execute();

    //Then: WriteCompletion should return success
    ASSERT_EQ(actual, true);
}

TEST(WriteCompletion, _RequestFlush_FlushError)
{
    //Given
    bool actual;
    auto mockVolIo = new NiceMock<MockVolumeIo>(nullptr, 1, "");
    VolumeIoSmartPtr volIo = VolumeIoSmartPtr(mockVolIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager("");
    NiceMock<MockStripe> mockStripe;
    StripeAddr stripeAddr;
    Stripe stripe(nullptr, true);
    VirtualBlkAddr startVsa;

    ON_CALL(*mockVolIo, GetLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolIo, GetVsa()).WillByDefault(ReturnRef(startVsa));

    //When: Execute WriteCompletion causing flush returning failure
    ON_CALL(mockStripe, DecreseBlksRemaining(_)).WillByDefault(Return(0));
    ON_CALL(mockStripe, Flush(_)).WillByDefault(Return(-1));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&mockStripe));
    WriteCompletion writeCompletion(volIo, &mockIWBStripeAllocator);
    actual = writeCompletion.Execute();

    //Then: WriteCompletion should return success
    ASSERT_EQ(actual, true);
}

} // namespace pos
