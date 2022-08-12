#include "src/logger/logger.h"
#include "src/meta_file_intf/mock_file_intf.h"

#include "test/integration-tests/mapper/reversemap/reverse_map_it_test.h"

namespace pos
{
void
ReverseMapTest::SetUp(void)
{
    _PrepareFile();
    reverseMapPack = new ReverseMapPack();
    reverseMapPack->Init(revMapWholefile, UT_WBLSID, 0, 4032, 1);
}

void
ReverseMapTest::TearDown(void)
{
    revMapWholefile->Close();
    delete reverseMapPack;
    reverseMapPack = nullptr;
}

MetaFileIntf*
ReverseMapTest::_PrepareFile(void)
{
    revMapWholefile = new MockFileIntf("RevMapWhole.bin");
    if (revMapWholefile->DoesFileExist() == false)
    {
        uint64_t fileSize = ReverseMapPack::GetfileSizePerStripe() * MAX_UT_VSID;
        POS_TRACE_INFO(EID(REVMAP_FILE_SIZE),
            "fileSizePerStripe:{}  maxVsid:{}  fileSize:{} for RevMapWhole",
            ReverseMapPack::GetfileSizePerStripe(), MAX_UT_VSID, fileSize);

        int ret = revMapWholefile->Create(fileSize);
        if (ret != 0)
        {
            POS_TRACE_ERROR(EID(REVMAP_FILE_SIZE),
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

TEST_F(ReverseMapTest, SetAndFlush)
{
    int ret = 0;
    // Set some revesemap data
    ret = reverseMapPack->SetReverseMapEntry(UT_OFFSET, UT_RBA, UT_VOLUMEID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    // Flush
    Stripe stripeDummy;
    uint32_t offset = stripeDummy.GetVsid() * reverseMapPack->GetfileSizePerStripe();
    ret = reverseMapPack->Flush(&stripeDummy, offset, nullptr);
    EXPECT_EQ(ret, RET_INT_SUCCESS);
    usleep(1000);
    EXPECT_EQ(ret, RET_INT_SUCCESS);
}

TEST_F(ReverseMapTest, LoadAndGet)
{
    int ret = 0;

    // Load
    ret = reverseMapPack->Load(0, nullptr);
    EXPECT_EQ(ret, RET_INT_SUCCESS);
    usleep(1000);

    // Get
    BlkAddr rba;
    uint32_t volumeId;
    std::tie(rba, volumeId) = reverseMapPack->GetReverseMapEntry(UT_OFFSET);
    EXPECT_EQ(rba, UT_RBA);
    EXPECT_EQ(volumeId, UT_VOLUMEID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);
}

}   // namespace pos
