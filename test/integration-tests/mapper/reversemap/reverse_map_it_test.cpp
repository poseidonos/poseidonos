#include "src/logger/logger.h"
#include "src/meta_file_intf/mock_file_intf.h"

#include "test/integration-tests/mapper/reversemap/reverse_map_it_test.h"

namespace pos
{
void
ReverseMapTest::SetUp(void)
{
    ReverseMapPack::SetPageSize();
    ReverseMapPack::SetNumPagesUT(UT_REVMAP_PAGES_PER_STRIPE);
    reverseMapPack = new ReverseMapPack();
    reverseMapPack->Init(UT_WBLSID);
}

void
ReverseMapTest::TearDown(void)
{
    delete reverseMapPack;
    reverseMapPack = nullptr;
}

MetaFileIntf*
ReverseMapTest::_PrepareFile(void)
{
    MetaFileIntf* revMapWholefile = new MockFileIntf("RevMapWhole.bin");
    if (revMapWholefile->DoesFileExist() == false)
    {
        uint64_t fileSize = ReverseMapPack::GetfileSizePerStripe() * MAX_UT_VSID;
        POS_TRACE_INFO((int)POS_EVENT_ID::REVMAP_FILE_SIZE,
            "fileSizePerStripe:{}  maxVsid:{}  fileSize:{} for RevMapWhole",
            ReverseMapPack::GetfileSizePerStripe(), MAX_UT_VSID, fileSize);

        int ret = revMapWholefile->Create(fileSize);
        if (ret != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::REVMAP_FILE_SIZE,
                "RevMapWhole file Create failed, ret:{}", ret);
            assert(false);
        }
    }
    revMapWholefile->Open();

    return revMapWholefile;
}

TEST_F(ReverseMapTest, GetFileSizePerStripe)
{
    uint64_t fileSizePerStripe = reverseMapPack->GetfileSizePerStripe();
    std::cout << "fileSizePerStripe:" << fileSizePerStripe << std::endl;

    EXPECT_EQ(fileSizePerStripe, DEFAULT_REVMAP_PAGE_SIZE * UT_REVMAP_PAGES_PER_STRIPE);
}

TEST_F(ReverseMapTest, LinkVSID)
{
    int ret = 0;

    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_LT(ret, RET_INT_SUCCESS);
}

TEST_F(ReverseMapTest, UnLinkVSID)
{
    int ret = 0;

    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    ret = reverseMapPack->UnLinkVsid();
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    ret = reverseMapPack->UnLinkVsid();
    EXPECT_LT(ret, RET_INT_SUCCESS);
}

TEST_F(ReverseMapTest, SetAndFlush)
{
    int ret = 0;

    // Prepare Mock file
    MetaFileIntf* revMapFile = _PrepareFile();
    reverseMapPack->SetRevMapFileUT(revMapFile);

    // Set some revesemap data
    ret = reverseMapPack->SetReverseMapEntry(UT_OFFSET, UT_RBA, UT_VOLUMEID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    // Link VSID
    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    // Flush
    Stripe stripeDummy;
    ret = reverseMapPack->Flush(&stripeDummy, nullptr);
    EXPECT_EQ(ret, RET_INT_SUCCESS);
    usleep(1000);

    ret = revMapFile->Close();
    EXPECT_EQ(ret, RET_INT_SUCCESS);
}

TEST_F(ReverseMapTest, LoadAndGet)
{
    int ret = 0;

    // Prepare Mock file
    MetaFileIntf* revMapFile = _PrepareFile();
    reverseMapPack->SetRevMapFileUT(revMapFile);

    // Link VSID
    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    // Load
    ret = reverseMapPack->Load();
    EXPECT_EQ(ret, RET_INT_SUCCESS);
    usleep(1000);

    // Get
    BlkAddr rba;
    uint32_t volumeId;
    std::tie(rba, volumeId) = reverseMapPack->GetReverseMapEntry(UT_OFFSET);
    EXPECT_EQ(rba, UT_RBA);
    EXPECT_EQ(volumeId, UT_VOLUMEID);

    ret = revMapFile->Close();
    EXPECT_EQ(ret, RET_INT_SUCCESS);
}

}   // namespace pos
