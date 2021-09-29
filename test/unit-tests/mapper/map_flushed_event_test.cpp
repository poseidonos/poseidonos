#include "src/mapper/map_flushed_event.h"

#include <gtest/gtest.h>

#include "test/unit-tests/mapper/i_map_manager_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(MapFlushedEvent, Execute_)
{
    // given
    NiceMock<MockIMapManagerInternal>* mapMan = new NiceMock<MockIMapManagerInternal>();
    MapFlushedEvent* e = new MapFlushedEvent(0, mapMan);
    // when
    EXPECT_CALL(*mapMan, MapFlushDone).Times(1);
    int ret = e->Execute();
    // then
    EXPECT_EQ(true, ret);
    delete mapMan;
    delete e;
}

} // namespace pos
