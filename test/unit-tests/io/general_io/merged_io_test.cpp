#include "src/io/general_io/merged_io.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/include/memory.h"
#include "src/io_submit_interface/io_submit_handler_status.h"
#include "src/state/state_manager.h"
#include "test/unit-tests/io_scheduler/io_dispatcher_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/include/i_array_device_mock.h"
#include "test/unit-tests/state/state_manager_mock.h"

using namespace pos;
using namespace std;
using ::testing::AtLeast;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(MergedIO, MergedIO_Constructor_One)
{
    // Given
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));

    //when
    MergedIO mergedIO(callback);

    //then : do noting
}

TEST(MergedIO, MergedIO_Constructor_Three)
{
    // Given
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockIODispatcher> mockIODispatcher;

    //when
    MergedIO mergedIO(callback, &mockIODispatcher, StateEnum::NORMAL);

    //then : do noting
}

TEST(MergedIO, Reset_)
{
    // Given
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));

    //when
    MergedIO mergedIO(callback);

    //then
    mergedIO.Reset();
}

TEST(MergedIO, AddContiguousBlock_)
{
    // Given
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));

    //when
    MergedIO mergedIO(callback);

    //then
    mergedIO.AddContiguousBlock();
}

TEST(MergedIO, SetNewStart_)
{
    // Given
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));

    //when
    MergedIO mergedIO(callback);
    void* newBuffer = nullptr;
    PhysicalBlkAddr newpba;
    newpba.lba = 0;

    //then
    mergedIO.SetNewStart(newBuffer, newpba);
}

TEST(MergedIO, IsContiguous_true)
{
    // Given
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));

    //when
    MergedIO mergedIO(callback);

    void* newBuffer = nullptr;
    NiceMock<MockIArrayDevice> mockIArrayDevice;
    PhysicalBlkAddr physicalBlkAddr{0, &mockIArrayDevice};
    PhysicalBlkAddr newPhysicalBlkAddr{8, &mockIArrayDevice};

    mergedIO.SetNewStart(newBuffer, physicalBlkAddr);
    bool actual, expected{true};

    //then
    actual = mergedIO.IsContiguous(newPhysicalBlkAddr);

    ASSERT_EQ(expected, actual);
}

TEST(MergedIO, IsContiguous_false)
{
    // Given
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));

    //when
    MergedIO mergedIO(callback);

    void* newBuffer = nullptr;
    NiceMock<MockIArrayDevice> mockIArrayDevice;
    PhysicalBlkAddr physicalBlkAddr{0, &mockIArrayDevice};
    PhysicalBlkAddr newPhysicalBlkAddr{31, &mockIArrayDevice};

    mergedIO.SetNewStart(newBuffer, physicalBlkAddr);
    bool actual, expected{false};

    //then
    actual = mergedIO.IsContiguous(newPhysicalBlkAddr);

    ASSERT_EQ(expected, actual);
}

TEST(MergedIO, Process_success)
{
    // Given
    void* buffer = Memory<SECTOR_SIZE>::Alloc(512 / SECTOR_SIZE);

    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));    
    NiceMock<MockIArrayDevice> mockIArrayDevice;
    //NiceMock<MockIODispatcher> mockIODispatcher;
    NiceMock<MockIODispatcher>* mockIODispatcher = new NiceMock<MockIODispatcher>;

    PhysicalBlkAddr physicalBlkAddr{0, &mockIArrayDevice};

    //when
    ON_CALL(*mockIODispatcher, Submit(_, _, _)).WillByDefault(Return(0));

    MergedIO mergedIO(callback, mockIODispatcher);
    mergedIO.SetNewStart(buffer, physicalBlkAddr);
    IOSubmitHandlerStatus actual, expected{IOSubmitHandlerStatus::SUCCESS};

    //then
    actual = mergedIO.Process(0);

    //ASSERT_EQ(expected, actual);

    Memory<SECTOR_SIZE>::Free(buffer);

    delete mockIODispatcher;
}

TEST(MergedIO, Process_ExistError)
{
    // Given
    void* buffer = Memory<SECTOR_SIZE>::Alloc(512 / SECTOR_SIZE);

    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));    
    NiceMock<MockIArrayDevice> mockIArrayDevice;
    //NiceMock<MockIODispatcher> mockIODispatcher;
    NiceMock<MockIODispatcher>* mockIODispatcher = new NiceMock<MockIODispatcher>;

    PhysicalBlkAddr physicalBlkAddr{0, &mockIArrayDevice};

    //when
    ON_CALL(*mockIODispatcher, Submit(_, _, _)).WillByDefault(Return(0));

    MergedIO mergedIO(callback, mockIODispatcher, StateEnum::STOP);
    mergedIO.SetNewStart(buffer, physicalBlkAddr);
    IOSubmitHandlerStatus actual, expected{IOSubmitHandlerStatus::SUCCESS};

    //then
    actual = mergedIO.Process(0);

    //ASSERT_EQ(expected, actual);

    Memory<SECTOR_SIZE>::Free(buffer);

    delete mockIODispatcher;
}

} // namespace pos
