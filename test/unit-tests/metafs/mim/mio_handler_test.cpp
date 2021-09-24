#include "src/metafs/mim/mio_handler.h"
#include "src/metafs/mim/mpio_handler.h"
#include "test/unit-tests/metafs/mim/metafs_io_q_mock.h"
#include "test/unit-tests/metafs/mim/mio_pool_mock.h"
#include "test/unit-tests/metafs/mim/mpio_pool_mock.h"
#include "test/unit-tests/metafs/mim/mpio_handler_mock.h"
#include "test/unit-tests/metafs/mim/metafs_io_request_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"
#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_io_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_management_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_wbt_api_mock.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class MioHandlerTestFixture : public ::testing::Test
{
public:
    MioHandlerTestFixture(void)
    : ioSQ(nullptr),
      ioCQ(nullptr),
      doneQ(nullptr),
      bottomhalfHandler(nullptr),
      mioPool(nullptr),
      mpioPool(nullptr),
      arrayInfo(nullptr),
      mgmt(nullptr),
      ctrl(nullptr),
      wbt(nullptr),
      io(nullptr),
      metaFs(nullptr)
    {
    }

    virtual ~MioHandlerTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        const uint32_t POOL_SIZE = 1024;

        ioSQ = new NiceMock<MockMetaFsIoQ<MetaFsIoRequest*>>;
        ioCQ = new NiceMock<MockMetaFsIoQ<Mio*>>;
        doneQ = new MockMetaFsIoQ<Mpio*>();
        bottomhalfHandler = new NiceMock<MockMpioHandler>(0, 0, doneQ);
        mpioPool = new NiceMock<MockMpioPool>(POOL_SIZE);
        mioPool = new NiceMock<MockMioPool>(mpioPool, POOL_SIZE);
        arrayInfo = new MockIArrayInfo();
        EXPECT_CALL(*arrayInfo, GetName).WillRepeatedly(Return("TESTARRAY"));
        EXPECT_CALL(*arrayInfo, GetIndex).WillRepeatedly(Return(0));

        mgmt = new MockMetaFsManagementApi(arrayInfo->GetIndex());
        ctrl = new MockMetaFsFileControlApi();
        wbt = new MockMetaFsWBTApi(arrayInfo->GetIndex(), ctrl);
        io = new MockMetaFsIoApi(arrayInfo->GetIndex(), ctrl);
        metaFs = new MockMetaFs(arrayInfo, false, mgmt, ctrl, io, wbt, nullptr);

        handler = new MioHandler(0, 0, ioSQ, ioCQ, mpioPool, mioPool);
    }

    virtual void
    TearDown(void)
    {
        delete handler;
        delete metaFs;
        delete arrayInfo;
        delete bottomhalfHandler;
    }

protected:
    MioHandler* handler;

    NiceMock<MockMetaFsIoQ<MetaFsIoRequest*>>* ioSQ;
    NiceMock<MockMetaFsIoQ<Mio*>>* ioCQ;
    MockMetaFsIoQ<Mpio*>* doneQ;
    NiceMock<MockMpioHandler>* bottomhalfHandler;
    NiceMock<MockMioPool>* mioPool;
    NiceMock<MockMpioPool>* mpioPool;

    MockIArrayInfo* arrayInfo;
    MockMetaFsManagementApi* mgmt;
    MockMetaFsFileControlApi* ctrl;
    MockMetaFsWBTApi* wbt;
    MockMetaFsIoApi* io;

    MockMetaFs* metaFs;
};

TEST_F(MioHandlerTestFixture, Normal)
{
    const int MAX_COUNT = 200;

    bool result = false;

    EXPECT_CALL(*ioSQ, Enqueue).WillRepeatedly(Return(true));
    EXPECT_CALL(*doneQ, Init);
    EXPECT_CALL(*ctrl, GetMaxMetaLpn).WillRepeatedly(Return(100));

    handler->BindPartialMpioHandler(bottomhalfHandler);
    result = handler->AddArrayInfo(arrayInfo->GetIndex());
    EXPECT_TRUE(result);

    MockMetaFsIoRequest* msg = new MockMetaFsIoRequest();

    for (int i = 0; i < MAX_COUNT; i++)
    {
        result = handler->EnqueueNewReq(msg);
        EXPECT_TRUE(result);
    }

    for (int i = 0; i < MAX_COUNT; i++)
    {
        handler->TophalfMioProcessing();
    }

    result = handler->RemoveArrayInfo(arrayInfo->GetIndex());
    EXPECT_TRUE(result);

    delete msg;
}

TEST_F(MioHandlerTestFixture, Repeat_AddAndRemoveArray)
{
    const int MAX_COUNT = 200;
    bool result = false;

    EXPECT_CALL(*arrayInfo, GetName).WillRepeatedly(Return("TESTARRAY"));
    EXPECT_CALL(*ctrl, GetMaxMetaLpn).WillRepeatedly(Return(100));

    result = handler->AddArrayInfo(arrayInfo->GetIndex());
    EXPECT_TRUE(result);

    result = handler->RemoveArrayInfo(arrayInfo->GetIndex());
    EXPECT_TRUE(result);

    result = handler->RemoveArrayInfo(arrayInfo->GetIndex());
    EXPECT_FALSE(result);
}

} // namespace pos
