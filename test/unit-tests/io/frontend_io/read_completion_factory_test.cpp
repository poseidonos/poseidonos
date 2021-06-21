#include "src/io/frontend_io/read_completion_factory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/event_scheduler/callback_factory.h"
#include "src/io/frontend_io/read_completion.h"
#include "test/unit-tests/bio/volume_io_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{

TEST(ReadCompletionFactory, ReadCompletionFactory_Create_)
{
    // Given
    void* buffer = nullptr;
    uint64_t sectorSize = 1;
    std::string arr_name = "";

    NiceMock<MockVolumeIo>* mockIovolumeIo = new NiceMock<MockVolumeIo>(buffer, sectorSize, 0);
    VolumeIoSmartPtr volumeIo(mockIovolumeIo);
    ReadCompletionFactory readCompletionFactory;
    bool actual, expected;

    // When: Create

    if (nullptr != readCompletionFactory.Create(volumeIo))
    {
        actual = true;
    }

    // Then: Receive result as true
    expected = true;
    ASSERT_EQ(expected, actual);
}

} // namespace pos
