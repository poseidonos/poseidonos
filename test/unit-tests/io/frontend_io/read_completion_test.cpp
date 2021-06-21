#include "src/io/frontend_io/read_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/bio/volume_io.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/lib/block_alignment.h"
#include "src/logger/logger.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator_service/allocator_service_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::Matcher;
using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
TEST(ReadCompletion, ReadCompletion_Constructor_OneArgument_Stack)
{
    // Given
    void* buffer = nullptr;
    uint64_t sectorSize = 1;

    NiceMock<MockVolumeIo>* mockIovolumeIo = new NiceMock<MockVolumeIo>(buffer, sectorSize, 0);
    VolumeIoSmartPtr volumeIo(mockIovolumeIo);

    // When: Try to Create New FlushReadCompletion object with 1 argument
    ReadCompletion readCompletion(volumeIo);

    // Then: Do nothing
}

TEST(ReadCompletion, ReadCompletion_Constructor_OneArgument_Heap)
{
    // Given
    void* buffer = nullptr;
    uint64_t sectorSize = 1;

    NiceMock<MockVolumeIo>* mockIovolumeIo = new NiceMock<MockVolumeIo>(buffer, sectorSize, 0);
    VolumeIoSmartPtr volumeIo(mockIovolumeIo);

    // When: Try to Create New FlushReadCompletion object with 1 argument
    ReadCompletion* readCompletion = new ReadCompletion(volumeIo);

    // Then: Release memory
    delete readCompletion;
}

TEST(ReadCompletion, ReadCompletion_DoSpecificJob_volumeIoNull)
{
    // Given
    void* buffer = nullptr;
    uint64_t sectorSize = 1;

    NiceMock<MockVolumeIo>* mockIovolumeIo = nullptr;
    VolumeIoSmartPtr volumeIo(mockIovolumeIo);
    bool actual, expected;

    // When: Try to input volumeio null
    ReadCompletion readCompletion(volumeIo);
    expected = true;
    actual = readCompletion.Execute();

    // Then:
    ASSERT_EQ(expected, actual);
}

TEST(ReadCompletion, ReadCompletion_DoSpecificJob_ExistErrorCount)
{
    // Given
    void* buffer = nullptr;
    uint64_t sectorSize = 1;

    VolumeIoSmartPtr volumeIo(new VolumeIo(buffer, sectorSize, 0));
    StripeAddr returnLsidEntry;
    returnLsidEntry.stripeId = 0;

    volumeIo->SetLsidEntry(returnLsidEntry);
    bool actual, expected;

    // When: Try to existed error
    ReadCompletion readCompletion(volumeIo);
    expected = true;

    readCompletion.InformError(IOErrorType::GENERIC_ERROR);

    actual = readCompletion.Execute();

    // Then:
    ASSERT_EQ(expected, actual);
}

TEST(ReadCompletion, ReadCompletion_DoSpecificJob_Success)
{
    // Given
    void* buffer = nullptr;
    uint64_t sectorSize = 1;

    VolumeIoSmartPtr volumeIo(new VolumeIo(buffer, sectorSize, 0));
    StripeAddr returnLsidEntry;
    returnLsidEntry.stripeId = 0;

    NiceMock<MockAllocatorService> mockAllocatorService;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;

    bool actual, expected;

    // When: Try to input volumeio null
    expected = true;
    ReadCompletion readCompletion(volumeIo);

    volumeIo->SetLsidEntry(returnLsidEntry);

    ON_CALL(mockAllocatorService, GetIWBStripeAllocator(Matcher<int>(_))).WillByDefault(Return(&mockIWBStripeAllocator));

    actual = readCompletion.Execute();

    // Then:
    ASSERT_EQ(expected, actual);
}

} // namespace pos
