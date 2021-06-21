#include "src/io/general_io/merger.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/io/frontend_io/read_completion_factory.h"
#include "test/unit-tests/array/array_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/include/i_array_device_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(Merger, Merger_Constructor_Stack_TwoArguments)
{
    //Given
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    ReadCompletionFactory readCompletionFactory;

    //When: create Nerger (in stack)
    Merger merger(volumeIo, &readCompletionFactory);

    //Then: Do nothing
}

TEST(Merger, Merger_Constructor_Heap)
{
    //Given
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    ReadCompletionFactory readCompletionFactory;

    //When: create merger (in heap)
    Merger* merger = new Merger(volumeIo, &readCompletionFactory);
    delete merger;

    //Then: Do nothing
}

TEST(Merger, Merger_Add_ContiguousPba)
{
    //Given
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    ReadCompletionFactory readCompletionFactory;

    PhysicalBlkAddr pba{0, nullptr};
    PhysicalBlkAddr pba2{8, nullptr};
    NiceMock<MockIArrayDevice> mockArrayDevice;
    PhysicalBlkAddr physicalBlkAddr{0, &mockArrayDevice};
    VirtualBlkAddr vsa{0, 0};
    StripeAddr lsidEntry{IN_USER_AREA, 0};

    //When: Add first pba
    Merger merger(volumeIo, &readCompletionFactory);
    merger.Add(pba, vsa, lsidEntry, 4096);

    //When: Add mergable pba
    merger.Add(pba2, vsa, lsidEntry, 4096);

    //Then: No split occurs
    ASSERT_EQ(0, merger.GetSplitCount());
}

TEST(Merger, Merger_Add_DiscretePba)
{
    //Given
    ReadCompletionFactory readCompletionFactory;
    MockVolumeIo* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, 2, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    MockVolumeIo* mockVolumeIoSplit = new NiceMock<MockVolumeIo>((void*)0xff00, 2, 0);
    VolumeIoSmartPtr volumeIoSplit(mockVolumeIoSplit);

    PhysicalBlkAddr pba{0, nullptr};
    PhysicalBlkAddr pba2{10, nullptr};
    NiceMock<MockIArrayDevice> mockArrayDevice;
    PhysicalBlkAddr physicalBlkAddr{0, &mockArrayDevice};
    VirtualBlkAddr vsa{0, 0};
    StripeAddr lsidEntry{IN_USER_AREA, 0};

    ON_CALL(*mockVolumeIo, Split(_, _)).WillByDefault(Return(volumeIoSplit));

    //When: Add first pba
    Merger merger(volumeIo, &readCompletionFactory);
    merger.Add(pba, vsa, lsidEntry, 4096);

    //When: Add non-mergable pba
    merger.Add(pba2, vsa, lsidEntry, 4096);

    //Then: One split occurs
    ASSERT_EQ(1, merger.GetSplitCount());
    ASSERT_EQ(volumeIoSplit, merger.GetSplit(0));
}

TEST(Merger, Merger_Cut_)
{
    //Given
    MockVolumeIo* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, 2, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    MockVolumeIo* mockVolumeIoSplit = new NiceMock<MockVolumeIo>((void*)0xff00, 2, 0);
    VolumeIoSmartPtr volumeIoSplit(mockVolumeIoSplit);
    ReadCompletionFactory readCompletionFactory;
    PhysicalBlkAddr pba{0, nullptr};
    NiceMock<MockIArrayDevice> mockArrayDevice;
    PhysicalBlkAddr physicalBlkAddr{0, &mockArrayDevice};
    VirtualBlkAddr vsa{0, 0};
    StripeAddr lsidEntry{IN_USER_AREA, 0};

    ON_CALL(*mockVolumeIo, Split(_, _)).WillByDefault(Return(volumeIoSplit));

    //When: cut without merging
    Merger merger(volumeIo, &readCompletionFactory);
    merger.Cut();

    //Then: No split occurs
    ASSERT_EQ(0, merger.GetSplitCount());
    ASSERT_EQ(nullptr, merger.GetSplit(0));

    //When: Add pba and cut afterwards
    merger.Add(pba, vsa, lsidEntry, 4096);
    merger.Cut();

    //Then: One split occurs
    ASSERT_EQ(1, merger.GetSplitCount());
    ASSERT_EQ(volumeIoSplit, merger.GetSplit(0));
}

} // namespace pos
