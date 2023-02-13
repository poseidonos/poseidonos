#include "src/io/general_io/io_controller.h"
#include "src/io_dispatcher_service/io_dispatcher_Service.h"
#include "src/event_scheduler_service/event_scheduler_service.h"
#include <gtest/gtest.h>

#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/io_scheduler/io_dispatcher_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
class IOControllerChild : public IOController
{
public:
    IOControllerChild(void)
    {
    }

    explicit IOControllerChild(IODispatcher* _ioDispatcher)
    {
        ioDispatcher = _ioDispatcher;
    }

    ~IOControllerChild(void)
    {
    }

    void
    SendVolumeIo(VolumeIoSmartPtr volumeIo)
    {
        _SendVolumeIo(volumeIo);
    }
};

TEST(IOController, IOController_Stack)
{
    // Given:
    IOControllerChild ioControllerChild;

    // When:

    // Then: Do nothing
}

TEST(IOController, IOController_Heap)
{
    // Given:
    IOControllerChild* ioControllerChild = new IOControllerChild();

    // When:

    // Then : release memory
    delete ioControllerChild;
}

TEST(IOController, IOController_SendVolumeIo)
{
    // Given:
    NiceMock<MockEventScheduler> mockEventScheduler;
    EventSchedulerServiceSingleton::Instance()->Register(&mockEventScheduler);
    NiceMock<MockIODispatcher> mockIODispatcher;
    IoDispatcherServiceSingleton::Instance()->Register(&mockIODispatcher);
    VolumeIoSmartPtr volumeIo;
    IOControllerChild* ioControllerChild = new IOControllerChild(&mockIODispatcher);

    // When: set iodispatcher mock and call _SendVolumeIo
    ON_CALL(mockIODispatcher, Submit(_, _, _)).WillByDefault(Return(0));
    EXPECT_CALL(mockIODispatcher, Submit(_, _, _)).Times(1);
    ioControllerChild->SendVolumeIo(volumeIo);

    // Then: release memory
    delete ioControllerChild;
    EventSchedulerServiceSingleton::Instance()->Unregister();
    IoDispatcherServiceSingleton::Instance()->Unregister();
    EventSchedulerServiceSingleton::ResetInstance();
    IoDispatcherServiceSingleton::ResetInstance();
}

} // namespace pos
