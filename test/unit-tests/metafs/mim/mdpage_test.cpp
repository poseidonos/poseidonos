#include "src/metafs/include/metafs_service.h"
#include "src/metafs/mim/mdpage.h"
#include "src/include/memory.h"

#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MDPage, Mdpage_Normal)
{
    const size_t TEST_PAGE_SIZE = 4096;
    const MetaLpnType lpn = 10;
    const FileDescriptorType fd = 9;

    void* buf = pos::Memory<TEST_PAGE_SIZE>::Alloc(1);
    bool result = false;

    MockIArrayInfo* info = new MockIArrayInfo();
    EXPECT_CALL(*info, GetName()).Times(2);

    MockMetaFs* metaFs = new MockMetaFs(info, false);
    EXPECT_CALL(*metaFs, GetEpochSignature()).Times(2);

    MDPage* page = new MDPage(buf);
    EXPECT_NE(page->GetDataBuf(), nullptr);

    std::string arrayName = info->GetName();
    page->Make(lpn, fd, arrayName);

    uint32_t signature = page->GetMfsSignature();
    EXPECT_NE(signature, 0);

    result = page->CheckFileMismatch(fd);
    EXPECT_EQ(result, true);

    result = page->CheckLpnMismatch(lpn);
    EXPECT_EQ(result, true);

    result = page->CheckValid(arrayName);
    EXPECT_EQ(result, true);

    // null check
    page->AttachControlInfo();
    page->ClearCtrlInfo();

    delete metaFs;
    delete info;
    delete page;

    pos::Memory<TEST_PAGE_SIZE>::Free(buf);
}

} // namespace pos
