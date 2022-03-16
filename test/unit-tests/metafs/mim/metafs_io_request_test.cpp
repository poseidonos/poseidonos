#include "src/metafs/mim/metafs_io_request.h"
#include "test/unit-tests/metafs/mim/metafs_io_request_mock.h"
#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFsIoRequest, ConstructorAndDestructor)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();
    delete req;
}

TEST(MetaFsIoRequest, CopyMsg_SyncRequest_FromUserThread)
{
    MockMetaFsIoRequest* reqOrigin = new MockMetaFsIoRequest();
    MetaFsIoRequest* req = new MetaFsIoRequest();

    EXPECT_CALL(*reqOrigin, GetError).Times(1);
    reqOrigin->ioMode = MetaIoMode::Sync;
    reqOrigin->originalMsg = nullptr;

    req->CopyUserReqMsg(*reqOrigin);

    EXPECT_EQ(reqOrigin->reqType, req->reqType);
    EXPECT_EQ(reqOrigin->ioMode, req->ioMode);
    EXPECT_EQ(reqOrigin->isFullFileIo, req->isFullFileIo);
    EXPECT_EQ(reqOrigin->fd, req->fd);
    EXPECT_EQ(reqOrigin->arrayId, req->arrayId);
    EXPECT_EQ(reqOrigin->buf, req->buf);
    EXPECT_EQ(reqOrigin->byteOffsetInFile, req->byteOffsetInFile);
    EXPECT_EQ(reqOrigin->byteSize, req->byteSize);
    EXPECT_EQ(reqOrigin->targetMediaType, req->targetMediaType);
    EXPECT_EQ(reqOrigin->aiocb, req->aiocb);
    EXPECT_EQ(reqOrigin->tagId, req->tagId);
    EXPECT_EQ(reqOrigin->baseMetaLpn, req->baseMetaLpn);
    EXPECT_EQ(reqOrigin->ioMode, req->ioMode);
    EXPECT_EQ(reqOrigin->GetError(), req->GetError());
    EXPECT_EQ(reqOrigin->fileCtx, req->fileCtx);

    // from user thread to meta io scheduler
    EXPECT_EQ(reqOrigin, req->originalMsg);

    delete req;
    delete reqOrigin;
}

TEST(MetaFsIoRequest, CopyMsg_SyncRequest_FromScheduler)
{
    MockMetaFsIoRequest* reqOrigin = new MockMetaFsIoRequest();
    MetaFsIoRequest* req = new MetaFsIoRequest();

    EXPECT_CALL(*reqOrigin, GetError).Times(1);
    reqOrigin->ioMode = MetaIoMode::Sync;
    reqOrigin->originalMsg = reqOrigin;

    req->CopyUserReqMsg(*reqOrigin);

    EXPECT_EQ(reqOrigin->reqType, req->reqType);
    EXPECT_EQ(reqOrigin->ioMode, req->ioMode);
    EXPECT_EQ(reqOrigin->isFullFileIo, req->isFullFileIo);
    EXPECT_EQ(reqOrigin->fd, req->fd);
    EXPECT_EQ(reqOrigin->arrayId, req->arrayId);
    EXPECT_EQ(reqOrigin->buf, req->buf);
    EXPECT_EQ(reqOrigin->byteOffsetInFile, req->byteOffsetInFile);
    EXPECT_EQ(reqOrigin->byteSize, req->byteSize);
    EXPECT_EQ(reqOrigin->targetMediaType, req->targetMediaType);
    EXPECT_EQ(reqOrigin->aiocb, req->aiocb);
    EXPECT_EQ(reqOrigin->tagId, req->tagId);
    EXPECT_EQ(reqOrigin->baseMetaLpn, req->baseMetaLpn);
    EXPECT_EQ(reqOrigin->ioMode, req->ioMode);
    EXPECT_EQ(reqOrigin->GetError(), req->GetError());
    EXPECT_EQ(reqOrigin->fileCtx, req->fileCtx);

    // meta io scheduler to mio handler
    EXPECT_EQ(reqOrigin->originalMsg, req->originalMsg);

    delete req;
    delete reqOrigin;
}

TEST(MetaFsIoRequest, CopyMsg_AsyncRequest)
{
    MockMetaFsIoRequest* reqOrigin = new MockMetaFsIoRequest();
    MetaFsIoRequest* req = new MetaFsIoRequest();

    EXPECT_CALL(*reqOrigin, GetError).Times(1);
    reqOrigin->ioMode = MetaIoMode::Async;

    req->CopyUserReqMsg(*reqOrigin);

    EXPECT_EQ(reqOrigin->reqType, req->reqType);
    EXPECT_EQ(reqOrigin->ioMode, req->ioMode);
    EXPECT_EQ(reqOrigin->isFullFileIo, req->isFullFileIo);
    EXPECT_EQ(reqOrigin->fd, req->fd);
    EXPECT_EQ(reqOrigin->arrayId, req->arrayId);
    EXPECT_EQ(reqOrigin->buf, req->buf);
    EXPECT_EQ(reqOrigin->byteOffsetInFile, req->byteOffsetInFile);
    EXPECT_EQ(reqOrigin->byteSize, req->byteSize);
    EXPECT_EQ(reqOrigin->targetMediaType, req->targetMediaType);
    EXPECT_EQ(reqOrigin->aiocb, req->aiocb);
    EXPECT_EQ(reqOrigin->tagId, req->tagId);
    EXPECT_EQ(reqOrigin->baseMetaLpn, req->baseMetaLpn);
    EXPECT_EQ(reqOrigin->ioMode, req->ioMode);
    EXPECT_EQ(reqOrigin->GetError(), req->GetError());
    EXPECT_EQ(reqOrigin->fileCtx, req->fileCtx);

    delete req;
    delete reqOrigin;
}

TEST(MetaFsIoRequest, Validity_Negative0)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    // positive
    req->reqType = MetaIoRequestType::Read;
    req->ioMode = MetaIoMode::Max;
    req->fd = MetaFsCommonConst::INVALID_FD;
    req->buf = nullptr;
    req->targetMediaType = MetaStorageType::Max;

    EXPECT_EQ(req->IsValid(), false);

    delete req;
}

TEST(MetaFsIoRequest, Validity_Negative1)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    req->reqType = MetaIoRequestType::Max;
    // positive
    req->ioMode = MetaIoMode::Sync;
    req->fd = MetaFsCommonConst::INVALID_FD;
    req->buf = nullptr;
    req->targetMediaType = MetaStorageType::Max;

    EXPECT_EQ(req->IsValid(), false);

    delete req;
}

TEST(MetaFsIoRequest, Validity_Negative2)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    req->reqType = MetaIoRequestType::Max;
    req->ioMode = MetaIoMode::Max;
    // positive
    req->fd = 0;
    req->buf = nullptr;
    req->targetMediaType = MetaStorageType::Max;

    EXPECT_EQ(req->IsValid(), false);

    delete req;
}

TEST(MetaFsIoRequest, Validity_Negative3)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    req->reqType = MetaIoRequestType::Max;
    req->ioMode = MetaIoMode::Max;
    req->fd = MetaFsCommonConst::INVALID_FD;
    // positive
    req->buf = (void*)this;
    req->targetMediaType = MetaStorageType::Max;

    EXPECT_EQ(req->IsValid(), false);

    delete req;
}

TEST(MetaFsIoRequest, Validity_Negative4)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    req->reqType = MetaIoRequestType::Max;
    req->ioMode = MetaIoMode::Max;
    req->fd = MetaFsCommonConst::INVALID_FD;
    req->buf = nullptr;
    // positive
    req->targetMediaType = MetaStorageType::SSD;

    EXPECT_EQ(req->IsValid(), false);

    delete req;
}

TEST(MetaFsIoRequest, Validity_Positive0)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    // negative
    req->reqType = MetaIoRequestType::Max;
    req->ioMode = MetaIoMode::Async;
    req->fd = 0;
    req->buf = (void*)this;
    req->targetMediaType = MetaStorageType::Max;

    EXPECT_EQ(req->IsValid(), false);

    delete req;
}

TEST(MetaFsIoRequest, CheckSyncIo)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    req->ioMode = MetaIoMode::Async;
    EXPECT_EQ(req->IsSyncIO(), false);

    req->ioMode = MetaIoMode::Sync;
    EXPECT_EQ(req->IsSyncIO(), true);

    delete req;
}

TEST(MetaFsIoRequest, CheckIoDone)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    req->requestCount = 2;
    req->NotifyIoCompletionToClient();
    EXPECT_EQ(req->IsIoCompleted(), false);

    req->NotifyIoCompletionToClient();
    EXPECT_EQ(req->IsIoCompleted(), true);

    delete req;
}

TEST(MetaFsIoRequest, CheckErrors)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    req->SetError(true);
    EXPECT_EQ(req->GetError(), true);

    req->SetError(false);
    EXPECT_EQ(req->GetError(), false);

    delete req;
}

TEST(MetaFsIoRequest, CheckRetryFlag)
{
    MetaFsIoRequest* req = new MetaFsIoRequest();

    EXPECT_EQ(req->GetRetryFlag(), false);

    req->SetRetryFlag();
    EXPECT_EQ(req->GetRetryFlag(), true);

    delete req;
}

TEST(MetaFsIoRequest, GetLogString_testIfTheStringIsCorrect)
{
    MetaFsIoRequest req;
    std::string log;
    log.append("reqType: " + (int)req.reqType);
    log.append(", ioMode: " + (int)req.ioMode);
    log.append(", tagId: " + req.tagId);
    log.append(", fd: " + std::to_string(req.fd));
    log.append(", targetMediaType: " + (int)req.targetMediaType);
    log.append(", arrayId: " + std::to_string(req.arrayId));
    log.append(", byteOffsetInFile: " + std::to_string(req.byteOffsetInFile));
    log.append(", byteSize: " + std::to_string(req.byteSize));
    log.append(", priority: " + (int)req.priority);
    std::string result = req.GetLogString();
    EXPECT_EQ(result, log);
}
} // namespace pos
