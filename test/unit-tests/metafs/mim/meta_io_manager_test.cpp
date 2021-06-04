#include "src/metafs/mim/meta_io_manager.h"
#include "test/unit-tests/metafs/mim/metafs_io_request_mock.h"
#include "test/unit-tests/metafs/mim/metafs_io_scheduler_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include <gtest/gtest.h>
#include <string>

using ::testing::Return;

namespace pos
{
TEST(MetaIoManager, CheckModuleName)
{
    const std::string name = "Meta IO Manager";
    MetaIoManager* mgr = new MetaIoManager();
    EXPECT_EQ(mgr->GetModuleName(), name);
    delete mgr;
}

TEST(MetaIoManager, CheckSuccess)
{
    MetaIoManager* mgr = new MetaIoManager();
    EXPECT_TRUE(mgr->IsSuccess(POS_EVENT_ID::SUCCESS));
    EXPECT_FALSE(mgr->IsSuccess(POS_EVENT_ID::MFS_ERROR_MESSAGE));
    delete mgr;
}

TEST(MetaIoManager, CheckInit)
{
    MetaIoManager* mgr = new MetaIoManager();
    mgr->Init();
    delete mgr;
}

TEST(MetaIoManager, CheckSanity)
{
    MockMetaFsIoRequest* req = new MockMetaFsIoRequest();
    MetaIoManager* mgr = new MetaIoManager();

    EXPECT_EQ(mgr->CheckReqSanity(*req), POS_EVENT_ID::SUCCESS);

    delete mgr;
    delete req;
}

TEST(MetaIoManager, CheckProcess_AsyncRequest)
{
    MockMetaFsIoScheduler* scheduler = new MockMetaFsIoScheduler(0, 0, 0);
    EXPECT_CALL(*scheduler, EnqueueNewReq).WillRepeatedly(Return(true));

    MockMetaFsIoRequest* req = new MockMetaFsIoRequest();
    req->ioMode = MetaIoMode::Async;
    req->reqType = MetaIoRequestType::Read;
    EXPECT_CALL(*req, IsSyncIO).WillRepeatedly(Return(true));
    EXPECT_CALL(*req, IsIoCompleted).WillRepeatedly(Return(true));
    EXPECT_CALL(*req, GetError).WillRepeatedly(Return(false));

    MetaIoManager* mgr = new MetaIoManager(scheduler);
    mgr->Init();

    EXPECT_EQ(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    req->reqType = MetaIoRequestType::Write;
    EXPECT_EQ(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    EXPECT_CALL(*scheduler, EnqueueNewReq).WillRepeatedly(Return(false));
    EXPECT_NE(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    delete mgr;
    delete req;
    delete scheduler;
}

TEST(MetaIoManager, CheckProcess_SyncRequest)
{
    MockMetaFsIoScheduler* scheduler = new MockMetaFsIoScheduler(0, 0, 0);
    EXPECT_CALL(*scheduler, EnqueueNewReq).WillRepeatedly(Return(true));

    MockMetaFsIoRequest* req = new MockMetaFsIoRequest();
    req->ioMode = MetaIoMode::Sync;
    req->reqType = MetaIoRequestType::Read;
    EXPECT_CALL(*req, IsSyncIO).WillRepeatedly(Return(true));
    EXPECT_CALL(*req, IsIoCompleted).WillRepeatedly(Return(true));
    EXPECT_CALL(*req, GetError).WillRepeatedly(Return(false));

    MetaIoManager* mgr = new MetaIoManager(scheduler);
    mgr->Init();

    EXPECT_EQ(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    req->reqType = MetaIoRequestType::Write;
    EXPECT_EQ(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    EXPECT_CALL(*scheduler, EnqueueNewReq).WillRepeatedly(Return(false));
    EXPECT_NE(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    delete mgr;
    delete req;
    delete scheduler;
}

TEST(MetaIoManager, CheckMss)
{
    const std::string arrayName = "TESTARRAY";
    MockMetaStorageSubsystem* mss = new MockMetaStorageSubsystem(arrayName);
    MetaIoManager* mgr = new MetaIoManager();
    mgr->Init();

    mgr->SetMss(mss);
    EXPECT_EQ(mss, mgr->GetMss());

    delete mgr;
    delete mss;
}

TEST(MetaIoManager, CheckArray)
{
    const std::string arrayName = "TESTARRAY";
    MockMetaFsIoScheduler* scheduler = new MockMetaFsIoScheduler(0, 0, 0);
    EXPECT_CALL(*scheduler, AddArrayInfo).WillRepeatedly(Return(true));
    EXPECT_CALL(*scheduler, RemoveArrayInfo).WillRepeatedly(Return(true));

    MetaIoManager* mgr = new MetaIoManager(scheduler);
    mgr->Init();

    EXPECT_TRUE(mgr->AddArrayInfo(arrayName));
    EXPECT_TRUE(mgr->RemoveArrayInfo(arrayName));

    EXPECT_CALL(*scheduler, AddArrayInfo).WillRepeatedly(Return(false));
    EXPECT_CALL(*scheduler, RemoveArrayInfo).WillRepeatedly(Return(false));

    EXPECT_FALSE(mgr->AddArrayInfo(arrayName));
    EXPECT_FALSE(mgr->RemoveArrayInfo(arrayName));

    delete mgr;
    delete scheduler;
}

} // namespace pos
