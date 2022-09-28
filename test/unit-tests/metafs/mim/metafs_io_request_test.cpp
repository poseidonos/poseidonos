#include "src/metafs/mim/metafs_io_request.h"

#include <gtest/gtest.h>

#include <cstring>

#include "test/unit-tests/metafs/mim/metafs_io_request_mock.h"

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

    EXPECT_CALL(*reqOrigin, GetError).Times(1);
    reqOrigin->ioMode = MetaIoMode::Sync;
    reqOrigin->originalMsg = nullptr;

    MetaFsIoRequest* req = new MetaFsIoRequest(*reqOrigin);

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

    EXPECT_CALL(*reqOrigin, GetError).Times(1);
    reqOrigin->ioMode = MetaIoMode::Sync;
    reqOrigin->originalMsg = reqOrigin;

    MetaFsIoRequest* req = new MetaFsIoRequest(*reqOrigin);

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

    EXPECT_CALL(*reqOrigin, GetError).Times(1);
    reqOrigin->ioMode = MetaIoMode::Async;

    MetaFsIoRequest* req = new MetaFsIoRequest(*reqOrigin);

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

TEST(MetaFsIoRequest, GetStartLpn_testIfTheStartIsReturnedAccordingToByteOffset)
{
    // given
    MetaFileExtent extents;
    extents.SetStartLpn(0);
    extents.SetCount(1024);
    MetaFileContext fileCtx;
    fileCtx.chunkSize = 4032;
    fileCtx.fileBaseLpn = 0;
    fileCtx.extentsCount = 1;
    fileCtx.CopyExtentsFrom(&extents, fileCtx.extentsCount);
    MetaFsIoRequest* req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;

    // when
    req->byteOffsetInFile = 0;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetStartLpn(), 0);

    // when
    req->byteOffsetInFile = 2016;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetStartLpn(), 0);

    // when
    req->byteOffsetInFile = 4032;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetStartLpn(), 1);

    // when
    req->byteOffsetInFile = 4032 + 2016;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetStartLpn(), 1);

    delete req;
}

TEST(MetaFsIoRequest, GetStartLpn_testIfTheStartIsReturnedNotAccordingToByteSize)
{
    // given
    MetaFileExtent extents;
    extents.SetStartLpn(0);
    extents.SetCount(1024);
    MetaFileContext fileCtx;
    fileCtx.chunkSize = 4032;
    fileCtx.fileBaseLpn = 0;
    fileCtx.extentsCount = 1;
    fileCtx.CopyExtentsFrom(&extents, fileCtx.extentsCount);
    MetaFsIoRequest* req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;

    // when
    req->byteOffsetInFile = 0;
    req->byteSize = 2016;

    // then
    EXPECT_EQ(req->GetStartLpn(), 0);

    // when
    req->byteOffsetInFile = 0;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetStartLpn(), 0);

    // when
    req->byteOffsetInFile = 0;
    req->byteSize = 4032 + 2016;

    // then
    EXPECT_EQ(req->GetStartLpn(), 0);

    // when
    req->byteOffsetInFile = 4032;
    req->byteSize = 2016;

    // then
    EXPECT_EQ(req->GetStartLpn(), 1);

    // when
    req->byteOffsetInFile = 4032;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetStartLpn(), 1);

    // when
    req->byteOffsetInFile = 4032;
    req->byteSize = 4032 + 2016;

    // then
    EXPECT_EQ(req->GetStartLpn(), 1);

    delete req;
}

TEST(MetaFsIoRequest, GetStartLpn_testIfTheStartIsReturnedAccordingToByteOffsetInComplicatedCases)
{
    // given
    MetaFileExtent extents[3];
    extents[0].SetStartLpn(43082495);
    extents[0].SetCount(1048);
    extents[1].SetStartLpn(43085111);
    extents[1].SetCount(2088);
    extents[2].SetStartLpn(43089807);
    extents[2].SetCount(512);
    MetaFileContext fileCtx;
    fileCtx.chunkSize = 4032;
    fileCtx.fileBaseLpn = 43082495;
    fileCtx.extentsCount = 3;
    fileCtx.CopyExtentsFrom(extents, fileCtx.extentsCount);
    MetaFsIoRequest* req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    req->extents = req->fileCtx->extents;
    req->extentsCount = req->fileCtx->extentsCount;

    // when, 1st
    req->byteOffsetInFile = 4032; // from 2nd lpn (43082495 + 1)
    req->byteSize = 4032;         // to 2nd lpn (43082495 + 1)

    // then
    EXPECT_EQ(req->GetStartLpn(), 43082495 + 1);

    // when, 2nd
    req->byteOffsetInFile = 4032 * 2; // from 3rd lpn (43082495 + 2)
    req->byteSize = 4032 * 1024;      // to 1026th lpn (43082495 + 1025)

    // then
    EXPECT_EQ(req->GetStartLpn(), 43082495 + 2);

    // when, 3rd
    req->byteOffsetInFile = 4032 * 1026; // from 1027th lpn (43082495 + 1026)
    req->byteSize = 4032 * 1024;         // to 1002nd of next extent (43085111 + 1001)

    // then
    EXPECT_EQ(req->GetStartLpn(), 43083521);

    // when, 4th
    req->byteOffsetInFile = 4032 * 2050; // from 1003rd lpn of 2nd extent (43085111 + 1002)
    req->byteSize = 4032 * 1024;         // to 2026th of the extent (43085111 + 2025)

    // then
    EXPECT_EQ(req->GetStartLpn(), 43085111 + 1002);

    delete req;
}

TEST(MetaFsIoRequest, GetRequestLpnCount_testIfTheCountOfLpnIsCorrect)
{
    // given
    MetaFileContext fileCtx;
    MetaFsIoRequest* req = new MetaFsIoRequest;
    req->fileCtx = &fileCtx;
    fileCtx.chunkSize = 4032;

    // when
    req->byteOffsetInFile = 0;
    req->byteSize = 2016;

    // then
    EXPECT_EQ(req->GetRequestLpnCount(), 1);

    // when
    req->byteOffsetInFile = 2016;
    req->byteSize = 2016;

    // then
    EXPECT_EQ(req->GetRequestLpnCount(), 1);

    // when
    req->byteOffsetInFile = 0;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetRequestLpnCount(), 1);

    // when
    req->byteOffsetInFile = 2016;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetRequestLpnCount(), 2);

    // when
    req->byteOffsetInFile = 4032;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetRequestLpnCount(), 1);

    // when
    req->byteOffsetInFile = 4031;
    req->byteSize = 4032;

    // then
    EXPECT_EQ(req->GetRequestLpnCount(), 2);

    // when
    req->byteOffsetInFile = 4031;
    req->byteSize = 4033;

    // then
    EXPECT_EQ(req->GetRequestLpnCount(), 2);

    // when
    req->byteOffsetInFile = 4031;
    req->byteSize = 4034;

    // then
    EXPECT_EQ(req->GetRequestLpnCount(), 3);
}
} // namespace pos
