#include "src/io/general_io/command_timeout_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "lib/spdk/lib/nvme/nvme_internal.h"

#include "spdk/nvme_spec.h"
#include "src/array/array.h"
#include "src/device/device_manager.h"
#include "src/device/unvme/unvme_drv.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/event_scheduler/io_completer.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/main/poseidonos.h"
#include "src/spdk_wrapper/nvme.hpp"
#include "src/spdk_wrapper/abort_context.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "test/unit-tests/io_scheduler/io_dispatcher_mock.h"
#include "test/unit-tests/io_scheduler/io_worker_mock.h"
#include "test/unit-tests/array_mgmt/array_manager_mock.h"
#include "test/unit-tests/utils/mock_builder.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class CommandTimeoutHandlerFixture : public ::testing::Test
{
public:
    CommandTimeoutHandlerFixture(void)
    {
    }

    virtual ~CommandTimeoutHandlerFixture()
    {
    }

    static int
    NvmeGetReg4(struct spdk_nvme_ctrlr* ctrlr, uint32_t offset, uint32_t* value)
    {
        if (call_count++ % 2 == 0)
        {
            *value = 0;
            return 0;
        }
        return -1;
    }

    static void
    SetUpTestSuite(void)
    {
        if (!initialized)
        {
            strcpy((char*)ctrlrData.sn, "different");
            qpair.state = NVME_QPAIR_DISCONNECTING;
            ctrlr.adminq = &qpair;
            ctrlr.cdata = ctrlrData;
            strcpy(ctrlr.trid.trstring, "TCP");

            tcp_ops.ctrlr_get_reg_4 = NvmeGetReg4;
            strcpy(tcp_ops.name, "TCP");
            spdk_nvme_transport_register(&tcp_ops);
            initialized = true;
        }
    }

    virtual void
    SetUp(void)
    {
        MockAffinityManager mockAffinityMgr = BuildDefaultAffinityManagerMock();
        devMgr = DeviceManagerSingleton::Instance();
        cpu_set_t cpu_set;
        mockIOWorker = new NiceMock<MockIOWorker>(cpu_set, 0);
        ON_CALL(*mockIOWorker, EnqueueUbio).WillByDefault([](UbioSmartPtr ubio) {
            auto callback = ubio->GetCallback();
            callback->InformError(IOErrorType::GENERIC_ERROR);
            callback->Execute();
        });

        NiceMock<MockUBlockDevice>* dev = new NiceMock<MockUBlockDevice>("", 0, nullptr);
        ON_CALL(*dev, GetDedicatedIOWorker).WillByDefault(Return(mockIOWorker));
        EXPECT_CALL(*dev, GetName).WillRepeatedly(Return("dev"));
        EXPECT_CALL(*dev, GetSN).WillRepeatedly(Return("different"));
        UblockSharedPtr ublockShared(dev);
        mockArrayMgr = new NiceMock<MockArrayManager>(nullptr, nullptr, nullptr, nullptr, nullptr);
        devMgr->SetDeviceEventCallback(mockArrayMgr);
        devMgr->Initialize(&mockIODispatcher);
        devMgr->AttachDevice(ublockShared);
        call_count = 0;
    }

    virtual void
    TearDown(void)
    {
        DeviceManagerSingleton::ResetInstance();
        devMgr = nullptr;
        delete mockIOWorker;
        delete mockArrayMgr;
    }

protected:
    DeviceManager* devMgr;
    NiceMock<MockIOWorker>* mockIOWorker;
    NiceMock<MockIODispatcher> mockIODispatcher;
    NiceMock<MockArrayManager>* mockArrayMgr;
    static struct spdk_nvme_ctrlr_data ctrlrData;
    static struct spdk_nvme_ctrlr ctrlr;
    static struct spdk_nvme_qpair qpair;
    static struct spdk_nvme_transport_ops tcp_ops;
    static bool initialized;
    static int call_count;
    uint16_t cid{0};
};

struct spdk_nvme_ctrlr_data CommandTimeoutHandlerFixture::ctrlrData;
struct spdk_nvme_ctrlr CommandTimeoutHandlerFixture::ctrlr;
struct spdk_nvme_qpair CommandTimeoutHandlerFixture::qpair;
struct spdk_nvme_transport_ops CommandTimeoutHandlerFixture::tcp_ops;
bool CommandTimeoutHandlerFixture::initialized;
int CommandTimeoutHandlerFixture::call_count;

TEST_F(CommandTimeoutHandlerFixture, Constructor)
{
    // When : Constructor and Destructor
    CommandTimeoutHandler* commandTimeoutHandler = new CommandTimeoutHandler();
    delete commandTimeoutHandler;
    // Then : Nothing
}

TEST_F(CommandTimeoutHandlerFixture, IsPendingAbortZero)
{
    // When : Constructor
    CommandTimeoutHandler* commandTimeoutHandler = new CommandTimeoutHandler();
    // Then : Check Pending Abort Zero
    bool actual = commandTimeoutHandler->IsPendingAbortZero();
    bool expected = true;
    ASSERT_EQ(actual, expected);
    delete commandTimeoutHandler;
}

TEST_F(CommandTimeoutHandlerFixture, __AbortSubmitHandler_Constructor)
{
    // Given : Abort Context is initialized
    AbortContext abortContext(nullptr, nullptr, 0);
    // When : __AbortSubmitHandler Constructor, Destructor
    CommandTimeoutHandler::__AbortSubmitHandler* commandAbortEvent = new CommandTimeoutHandler::__AbortSubmitHandler(&abortContext, devMgr);
    delete commandAbortEvent;
    // Then : Nothing
}

TEST_F(CommandTimeoutHandlerFixture, __AbortSubmitHandler_DiskIO)
{
    // Given : Abort Context is initialized
    AbortContext* abortContext = new AbortContext(nullptr, nullptr, 0);
    // When : __AbortSubmitHandler Constructor
    CommandTimeoutHandler::__AbortSubmitHandler* commandAbortEvent = new CommandTimeoutHandler::__AbortSubmitHandler(abortContext, devMgr);
    DevName name("dev");
    UblockSharedPtr ptr(devMgr->GetDev(name));
    commandAbortEvent->DiskIO(ptr, nullptr);

    // Then : DiskIO Execute
    delete commandAbortEvent;
    delete abortContext;
}

TEST_F(CommandTimeoutHandlerFixture, DISABLED__AbortSubmitHandler_Execute)
{
    // When : Timeout occured (cfs is not set)
    ctrlr.is_failed = false;

    struct spdk_nvme_qpair qpair;
    Nvme::ControllerTimeoutCallback(nullptr, &ctrlr, &qpair, cid);
    // Then : Command Timeout Abort Handler is called and abort event is submitted
    std::queue<EventSmartPtr> queue =
        EventSchedulerSingleton::Instance()->DequeueEvents();
    EXPECT_GT(queue.size(), 0);
    EXPECT_FALSE(ctrlr.is_failed);

    // When : Timeout occured (cfs is not set) Cid is random value.
    ctrlr.is_failed = false;
    Nvme::ControllerTimeoutCallback(nullptr, &ctrlr, nullptr, 1);
    // Then : qpair is null and is_failed is set as true
    queue =
        EventSchedulerSingleton::Instance()->DequeueEvents();
    EXPECT_EQ(queue.size(), 0);
    EXPECT_TRUE(ctrlr.is_failed);

    // When : Timeout occured (cfs is set)
    Nvme::ControllerTimeoutCallback(nullptr, &ctrlr, &qpair, cid);
    // Then : Controller is failed due to reset handler being called
    EXPECT_TRUE(ctrlr.is_failed);

    // Given: Ctrl status went back to normal
    ctrlr.is_failed = true;
    // When : Timeout occured (cfs is not set but previous abort event is pending)
    Nvme::ControllerTimeoutCallback(nullptr, &ctrlr, &qpair, cid);
    // Then : Controller is failed due to reset handler being called
    EXPECT_TRUE(ctrlr.is_failed);

    // Given : Abort Context is initialized
    AbortContext* abortContext = new AbortContext(&ctrlr, &qpair, 0);
    // When : __AbortSubmitHandler is executed
    CommandTimeoutHandler::__AbortSubmitHandler* commandAbortEvent = new CommandTimeoutHandler::__AbortSubmitHandler(abortContext, devMgr);
    bool actual = commandAbortEvent->Execute();
    // Then: Abort execution succeeds
    ASSERT_EQ(actual, true);

    delete commandAbortEvent;
}

} // namespace pos
