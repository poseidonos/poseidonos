#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <fstream>

#include "src/allocator/context_manager/i_allocator_file_io_client.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/meta_file_intf/async_context.h"
#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/context_manager/context/context.h"
#include "src/allocator/address/allocator_address_info.h"

using namespace ::testing;
using namespace std;

using testing::NiceMock;

namespace pos
{

TEST(AllocatorFileIo, InitBeforeFlushAfterLoad_testIfSegmentCtxIsSerializedAndDeserializedProperly)
{
    // Given: 10 segments, all of which are in the same SSD state, but with different number of valid blocks
    int numSegments = 10;
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(numSegments);
    SegmentInfoData* segmentInfoDataArray = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoDataArray[i].Set(i /* valid count */, 1, SegmentState::SSD);
    }

    SegmentCtx* segCtxClient = new SegmentCtx(nullptr, nullptr, segmentInfoDataArray, nullptr, nullptr, &addrInfo, nullptr);

    // When 1: the segment context is initialized()
    segCtxClient->Init();

    // Then 1: the total size to flush should be the sum of segmentctx header and the list of segment info data
    auto actual = segCtxClient->GetTotalDataSize();
    uint64_t expected = sizeof(SegmentCtxHeader) + SegmentInfoData::ONSSD_SIZE * numSegments + SegmentCtxExtended::ONSSD_SIZE;
    EXPECT_EQ(expected, actual);

    // When 2: the segment context is flushed to a local file system
    std::string metafilePath = "/tmp/segment_ctx.bin";
    MockFileIntf* file = new MockFileIntf(metafilePath, 0, MetaFileType::General);
    file->Create(expected);
    file->Open();
    char* ioBuffer = new char[expected];
    for(int i=0; i<expected; i++) {
        ioBuffer[i] = 0;
    }
    segCtxClient->BeforeFlush(ioBuffer /* OUT */);
    int ret = file->IssueIO(MetaFsIoOpcode::Write, 0, expected, ioBuffer /* IN */);

    // Then 2: the size of the metafile (on file system) should be the same to "expected" size calculated as above
    EXPECT_EQ(0, ret);
    {
        ifstream is(metafilePath, ios::in | ios::ate);
        is.seekg (0, ios::end);
        uint64_t length = is.tellg();
        EXPECT_EQ(expected, length);
        is.close();
    }

    // When 3: the segment context is loaded from a local file system
    char* newIoBuffer = new char[expected];
    file->IssueIO(MetaFsIoOpcode::Read, 0, expected, newIoBuffer /* IN */);
    segCtxClient->AfterLoad(newIoBuffer);

    // Then 3: the segment info data should match with what we have submitted to the local file system through MockFileIntf
    //         Also, the segment context should
    EXPECT_EQ(0, ret);
    for(int i=0; i<expected; i++)
    {
        EXPECT_EQ(ioBuffer[i], newIoBuffer[i]);
    }
    for(int i=0; i<numSegments; i++)
    {
        EXPECT_EQ(i, segCtxClient->GetValidBlockCount(i));
        EXPECT_EQ(1, segCtxClient->GetOccupiedStripeCount(i));
        EXPECT_EQ(SegmentState::SSD, segCtxClient->GetSegmentState(i));
    }
}
}