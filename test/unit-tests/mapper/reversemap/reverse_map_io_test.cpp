#include "src/mapper/reversemap/reverse_map_io.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/mapper/reversemap/reverse_map_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;

namespace pos
{
TEST(ReverseMapIo, ReverseMapIo_testIfConstructedSuccessfully)
{
    {
        NiceMock<MockReverseMapPack> reverseMapPack;
        CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
        NiceMock<MockMetaFileIntf> file;
        NiceMock<MockTelemetryPublisher> tp;
        NiceMock<MockEventScheduler> es;

        uint64_t offset = 0;
        IoDirection dir = IoDirection::IO_FLUSH;

        ReverseMapIo reverseMapIo(&reverseMapPack, callback,
            &file, offset, dir, &tp, &es, [&](ReverseMapIo* revmap) {});
    }

    {
        NiceMock<MockReverseMapPack> reverseMapPack;
        CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
        NiceMock<MockMetaFileIntf> file;
        NiceMock<MockTelemetryPublisher> tp;
        NiceMock<MockEventScheduler> es;

        uint64_t offset = 0;
        IoDirection dir = IoDirection::IO_FLUSH;

        ReverseMapIo* reverseMapIo = new ReverseMapIo(&reverseMapPack, callback,
            &file, offset, dir, &tp, &es, [&](ReverseMapIo* revmap) {});
        delete reverseMapIo;
    }
}

TEST(ReverseMapIo, Load_testLoadSinglePage)
{
    NiceMock<MockReverseMapPack> reverseMapPack;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockMetaFileIntf> file;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockEventScheduler> es;

    uint32_t fd = 10;
    uint64_t offset = 4032;
    uint64_t length = 4032;
    IoDirection dir = IoDirection::IO_LOAD;

    ReverseMapIo* reverseMapIo = new ReverseMapIo(&reverseMapPack, callback,
        &file, offset, dir, &tp, &es, [&](ReverseMapIo* revmap) {});

    std::vector<ReverseMapPage> reverseMapPages = {ReverseMapPage{.pageNum = offset, .length = length, .buffer = nullptr}};
    ON_CALL(reverseMapPack, GetReverseMapPages).WillByDefault(Return(reverseMapPages));
    ON_CALL(file, GetFd).WillByDefault(Return(fd));

    RevMapPageAsyncIoCtx* req;
    EXPECT_CALL(file, AsyncIO).WillOnce(DoAll(SaveArg<0>(&req), Return(0)));

    int ret = reverseMapIo->Load();
    EXPECT_EQ(ret, 0);

    EXPECT_THAT(req->GetOpcode(), MetaFsIoOpcode::Read);
    EXPECT_THAT(req->GetFd(), fd);
    EXPECT_THAT(req->GetFileOffset(), offset);
    EXPECT_THAT(req->GetLength(), length);
    EXPECT_THAT(req->GetMpageNum(), 0);

    EXPECT_EQ(reverseMapIo->GetNumIssuedIoCnt(), 1);

    delete reverseMapIo;
    delete req;
}

TEST(ReverseMapIo, Load_testLoadRevmapPages)
{
    NiceMock<MockReverseMapPack> reverseMapPack;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockMetaFileIntf> file;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockEventScheduler> es;

    uint32_t fd = 10;
    uint64_t offset = 4032;
    uint64_t length = 4032;
    IoDirection dir = IoDirection::IO_LOAD;

    ReverseMapIo* reverseMapIo = new ReverseMapIo(&reverseMapPack, callback,
        &file, offset, dir, &tp, &es, [&](ReverseMapIo* revmap) {});

    uint64_t pageNum = 0;
    std::vector<ReverseMapPage> reverseMapPages = {
        ReverseMapPage{.pageNum = pageNum++, .length = length, .buffer = nullptr},
        ReverseMapPage{.pageNum = pageNum, .length = length, .buffer = nullptr}};
    ON_CALL(reverseMapPack, GetReverseMapPages).WillByDefault(Return(reverseMapPages));
    ON_CALL(file, GetFd).WillByDefault(Return(fd));

    RevMapPageAsyncIoCtx *req1, *req2;
    EXPECT_CALL(file, AsyncIO).WillOnce(DoAll(SaveArg<0>(&req1), Return(0))).WillOnce(DoAll(SaveArg<0>(&req2), Return(0)));

    int ret = reverseMapIo->Load();
    EXPECT_EQ(ret, 0);

    EXPECT_THAT(req1->GetOpcode(), MetaFsIoOpcode::Read);
    EXPECT_THAT(req1->GetFd(), fd);
    EXPECT_THAT(req1->GetFileOffset(), offset);
    EXPECT_THAT(req1->GetLength(), length);
    EXPECT_THAT(req1->GetMpageNum(), 0);

    EXPECT_THAT(req2->GetOpcode(), MetaFsIoOpcode::Read);
    EXPECT_THAT(req2->GetFd(), fd);
    EXPECT_THAT(req2->GetFileOffset(), offset + length);
    EXPECT_THAT(req2->GetLength(), length);
    EXPECT_THAT(req2->GetMpageNum(), 1);

    EXPECT_EQ(reverseMapIo->GetNumIssuedIoCnt(), 2);

    delete reverseMapIo;
    delete req1;
    delete req2;
}

TEST(ReverseMapIo, Flush_testFlushSinglePage)
{
    NiceMock<MockReverseMapPack> reverseMapPack;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockMetaFileIntf> file;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockEventScheduler> es;

    uint32_t fd = 10;
    uint64_t offset = 4032 * 3;
    uint64_t length = 4032;
    IoDirection dir = IoDirection::IO_LOAD;

    ReverseMapIo* reverseMapIo = new ReverseMapIo(&reverseMapPack, callback,
        &file, offset, dir, &tp, &es, [&](ReverseMapIo* revmap) {});

    std::vector<ReverseMapPage> reverseMapPages = {ReverseMapPage{.pageNum = offset, .length = length, .buffer = nullptr}};
    ON_CALL(reverseMapPack, GetReverseMapPages).WillByDefault(Return(reverseMapPages));
    ON_CALL(file, GetFd).WillByDefault(Return(fd));

    RevMapPageAsyncIoCtx* req;
    EXPECT_CALL(file, AsyncIO).WillOnce(DoAll(SaveArg<0>(&req), Return(0)));

    int ret = reverseMapIo->Flush();
    EXPECT_EQ(ret, 0);

    EXPECT_THAT(req->GetOpcode(), MetaFsIoOpcode::Write);
    EXPECT_THAT(req->GetFd(), fd);
    EXPECT_THAT(req->GetFileOffset(), offset);
    EXPECT_THAT(req->GetLength(), length);
    EXPECT_THAT(req->GetMpageNum(), 0);

    EXPECT_EQ(reverseMapIo->GetNumIssuedIoCnt(), 1);

    delete reverseMapIo;
    delete req;
}

TEST(ReverseMapIo, Flush_testLoadRevmapPages)
{
    NiceMock<MockReverseMapPack> reverseMapPack;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockMetaFileIntf> file;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockEventScheduler> es;

    uint32_t fd = 10;
    uint64_t offset = 4032 * 3;
    uint64_t length = 4032;
    IoDirection dir = IoDirection::IO_LOAD;

    ReverseMapIo* reverseMapIo = new ReverseMapIo(&reverseMapPack, callback,
        &file, offset, dir, &tp, &es, [&](ReverseMapIo* revmap) {});

    uint64_t pageNum = 0;
    std::vector<ReverseMapPage> reverseMapPages = {
        ReverseMapPage{.pageNum = pageNum++, .length = length, .buffer = nullptr},
        ReverseMapPage{.pageNum = pageNum, .length = length, .buffer = nullptr}};
    ON_CALL(reverseMapPack, GetReverseMapPages).WillByDefault(Return(reverseMapPages));
    ON_CALL(file, GetFd).WillByDefault(Return(fd));

    RevMapPageAsyncIoCtx *req1, *req2;
    EXPECT_CALL(file, AsyncIO).WillOnce(DoAll(SaveArg<0>(&req1), Return(0))).WillOnce(DoAll(SaveArg<0>(&req2), Return(0)));

    int ret = reverseMapIo->Flush();
    EXPECT_EQ(ret, 0);

    EXPECT_THAT(req1->GetOpcode(), MetaFsIoOpcode::Write);
    EXPECT_THAT(req1->GetFd(), fd);
    EXPECT_THAT(req1->GetFileOffset(), offset);
    EXPECT_THAT(req1->GetLength(), length);
    EXPECT_THAT(req1->GetMpageNum(), 0);

    EXPECT_THAT(req2->GetOpcode(), MetaFsIoOpcode::Write);
    EXPECT_THAT(req2->GetFd(), fd);
    EXPECT_THAT(req2->GetFileOffset(), offset + length);
    EXPECT_THAT(req2->GetLength(), length);
    EXPECT_THAT(req2->GetMpageNum(), 1);

    EXPECT_EQ(reverseMapIo->GetNumIssuedIoCnt(), 2);

    delete reverseMapIo;
    delete req1;
    delete req2;
}

TEST(ReverseMapIo, _RevMapPageIoDone_testIfFlushCompletedSuccessfully)
{
    NiceMock<MockReverseMapPack> reverseMapPack;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockMetaFileIntf> file;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockEventScheduler> es;

    uint32_t fd = 10;
    uint64_t offset = 4032 * 3;
    uint64_t length = 4032;
    IoDirection dir = IoDirection::IO_FLUSH;

    ReverseMapIo* reverseMapIo = new ReverseMapIo(&reverseMapPack, callback,
        &file, offset, dir, &tp, &es, [&](ReverseMapIo* revmap) {});

    uint64_t pageNum = 0;
    std::vector<ReverseMapPage> reverseMapPages = {
        ReverseMapPage{.pageNum = pageNum++, .length = length, .buffer = nullptr},
        ReverseMapPage{.pageNum = pageNum, .length = length, .buffer = nullptr}};
    ON_CALL(reverseMapPack, GetReverseMapPages).WillByDefault(Return(reverseMapPages));
    ON_CALL(file, GetFd).WillByDefault(Return(fd));

    EXPECT_CALL(file, AsyncIO).WillRepeatedly([&](AsyncMetaFileIoCtx* ctx) {
        (ctx->GetCallback())(ctx);
        return 0;
    });

    EXPECT_EQ(reverseMapIo->Flush(), 0);

    EXPECT_EQ(reverseMapIo->GetNumIssuedIoCnt(), 2);
    reverseMapIo->WaitPendingIoDone();
    EXPECT_EQ(reverseMapIo->GetMapFlushState(), MapFlushState::FLUSH_DONE);
}

TEST(ReverseMapIo, _RevMapPageIoDone_testIfLoadCompletedSuccessfully)
{
    NiceMock<MockReverseMapPack> reverseMapPack;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockMetaFileIntf> file;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockEventScheduler> es;

    uint32_t fd = 10;
    uint64_t offset = 4032 * 3;
    uint64_t length = 4032;
    IoDirection dir = IoDirection::IO_LOAD;

    ReverseMapIo* reverseMapIo = new ReverseMapIo(&reverseMapPack, callback,
        &file, offset, dir, &tp, &es, [&](ReverseMapIo* revmap) {});

    uint64_t pageNum = 0;
    std::vector<ReverseMapPage> reverseMapPages = {
        ReverseMapPage{.pageNum = pageNum++, .length = length, .buffer = nullptr},
        ReverseMapPage{.pageNum = pageNum, .length = length, .buffer = nullptr}};
    ON_CALL(reverseMapPack, GetReverseMapPages).WillByDefault(Return(reverseMapPages));
    ON_CALL(file, GetFd).WillByDefault(Return(fd));

    EXPECT_CALL(file, AsyncIO).WillRepeatedly([&](AsyncMetaFileIoCtx* ctx) {
        (ctx->GetCallback())(ctx);
        return 0;
    });

    EXPECT_CALL(reverseMapPack, HeaderLoaded).Times(1);
    EXPECT_EQ(reverseMapIo->Load(), 0);

    EXPECT_EQ(reverseMapIo->GetNumIssuedIoCnt(), 2);
    reverseMapIo->WaitPendingIoDone();
    EXPECT_EQ(reverseMapIo->GetMapFlushState(), MapFlushState::FLUSH_DONE);
}

} // namespace pos
