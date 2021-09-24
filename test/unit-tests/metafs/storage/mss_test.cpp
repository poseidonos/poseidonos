#include "src/metafs/storage/mss.h"

#include <gtest/gtest.h>

namespace pos
{
class MssTester : public MetaStorageSubsystem
{
public:
    explicit MssTester(int arrayId)
    : MetaStorageSubsystem(arrayId)
    {
    }
    ~MssTester(void)
    {
    }
    POS_EVENT_ID CreateMetaStore(int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag = false)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    POS_EVENT_ID Open(void)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    POS_EVENT_ID Close(void)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    uint64_t GetCapacity(MetaStorageType mediaType)
    {
        return 1024;
    }
    POS_EVENT_ID ReadPage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages)
    {
        return POS_EVENT_ID::MFS_ARRAY_ADD_FAILED;
    }
    POS_EVENT_ID WritePage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages)
    {
        return POS_EVENT_ID::MFS_ARRAY_DELETE_FAILED;
    }
    bool IsAIOSupport(void)
    {
        return true;
    }
    POS_EVENT_ID ReadPageAsync(MssAioCbCxt* cb)
    {
        return POS_EVENT_ID::MFS_SYSTEM_MOUNT_AGAIN;
    }
    POS_EVENT_ID WritePageAsync(MssAioCbCxt* cb)
    {
        return POS_EVENT_ID::MFS_SYSTEM_OPEN_FAILED;
    }
    POS_EVENT_ID TrimFileData(MetaStorageType mediaType, MetaLpnType startLpn, void* buffer, MetaLpnType numPages)
    {
        return POS_EVENT_ID::MFS_ARRAY_CREATE_FAILED;
    }
    LogicalBlkAddr TranslateAddress(MetaStorageType type, MetaLpnType theLpn)
    {
        return {0, 0};
    }
};

TEST(MssTester, CheckRead)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Read;
    MetaStorageType mediaType = MetaStorageType::SSD;
    MetaLpnType metaLpn = 0;
    MetaLpnType numPages = 0;
    uint32_t mpio_id = 0;
    uint32_t tagid = 0;

    EXPECT_EQ(mss->DoPageIO(opcode, mediaType, metaLpn, nullptr, numPages,
                mpio_id, tagid), POS_EVENT_ID::MFS_ARRAY_ADD_FAILED);

    delete mss;
}

TEST(MssTester, CheckWrite)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Write;
    MetaStorageType mediaType = MetaStorageType::SSD;
    MetaLpnType metaLpn = 0;
    MetaLpnType numPages = 0;
    uint32_t mpio_id = 0;
    uint32_t tagid = 0;

    EXPECT_EQ(mss->DoPageIO(opcode, mediaType, metaLpn, nullptr, numPages,
                mpio_id, tagid), POS_EVENT_ID::MFS_ARRAY_DELETE_FAILED);

    delete mss;
}

TEST(MssTester, CheckTrim)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Trim;
    MetaStorageType mediaType = MetaStorageType::SSD;
    MetaLpnType metaLpn = 0;
    MetaLpnType numPages = 0;
    uint32_t mpio_id = 0;
    uint32_t tagid = 0;

    EXPECT_EQ(mss->DoPageIO(opcode, mediaType, metaLpn, nullptr, numPages,
                mpio_id, tagid), POS_EVENT_ID::MFS_ARRAY_CREATE_FAILED);

    delete mss;
}

TEST(MssTester, CheckRead_Async)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Read;

    EXPECT_EQ(mss->DoPageIOAsync(opcode, nullptr), POS_EVENT_ID::MFS_SYSTEM_MOUNT_AGAIN);

    delete mss;
}

TEST(MssTester, CheckWrite_Async)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Write;

    EXPECT_EQ(mss->DoPageIOAsync(opcode, nullptr), POS_EVENT_ID::MFS_SYSTEM_OPEN_FAILED);

    delete mss;
}

TEST(MssTester, CheckTranslate)
{
    MssTester* mss = new MssTester(0);

    EXPECT_EQ(mss->TranslateAddress(MetaStorageType::SSD, 0).stripeId, 0);
    EXPECT_EQ(mss->TranslateAddress(MetaStorageType::SSD, 0).offset, 0);

    delete mss;
}
} // namespace pos
