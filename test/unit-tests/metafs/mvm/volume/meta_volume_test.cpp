#include "src/metafs/mvm/volume/meta_volume.h"
#include "src/metafs/mvm/volume/meta_volume_state.h"

#include "test/unit-tests/metafs/mvm/volume/file_descriptor_allocator_mock.h"
#include "test/unit-tests/metafs/mvm/volume/extent_allocator_mock.h"
#include "test/unit-tests/metafs/mvm/volume/catalog_manager_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_manager_mock.h"
#include "test/unit-tests/metafs/mvm/volume/catalog_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <map>
#include <string>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;

using namespace std;

namespace pos
{
class MetaVolumeTester : public MetaVolume
{
public:
    MetaVolumeTester(int arrayId, MetaVolumeType volumeType,
            MetaLpnType maxVolumePageNum = 0, InodeManager* inodeMgr = nullptr,
            CatalogManager* catalogMgr = nullptr)
    : MetaVolume(arrayId, volumeType, maxVolumePageNum, inodeMgr, catalogMgr)
    {
    }
    virtual ~MetaVolumeTester(void)
    {
    }
    void InitVolumeBaseLpn(void)
    {
        volumeBaseLpn = 2000;
    }
    bool IsOkayToStore(FileSizeType fileByteSize, MetaFilePropertySet& prop)
    {
        return true;
    }
    void SetVolumeType(MetaVolumeType volumeType)
    {
        this->volumeType = volumeType;
    }
    void SetVolumeState(MetaVolumeState state)
    {
        this->volumeState = state;
    }
};

class MetaVolumeFixture : public ::testing::Test
{
public:
    MetaVolumeFixture(void)
    {
    }
    virtual ~MetaVolumeFixture(void)
    {
    }
    virtual void
    SetUp(void)
    {
        inodeMgr = new NiceMock<MockInodeManager>(arrayId);
        catalogMgr = new NiceMock<MockCatalogManager>(arrayId);
        metaStorage = new NiceMock<MockMetaStorageSubsystem>(arrayId);

        metaVolume = new MetaVolumeTester(arrayId, volumeType, maxVolumePageNum,
                                    inodeMgr, catalogMgr);

        EXPECT_CALL(*inodeMgr, Init);
        EXPECT_CALL(*inodeMgr, SetMss);
        EXPECT_CALL(*catalogMgr, Init);
        EXPECT_CALL(*catalogMgr, SetMss);

        EXPECT_CALL(*inodeMgr, GetMetaFileBaseLpn()).WillRepeatedly(Return(DEFAULT_SIZE));
        EXPECT_CALL(*inodeMgr, GetRegionSizeInLpn()).WillRepeatedly(Return(DEFAULT_SIZE));
        EXPECT_CALL(*catalogMgr, GetRegionSizeInLpn).WillRepeatedly(Return(DEFAULT_SIZE));

        metaVolume->Init(metaStorage);
    }
    virtual void
    TearDown(void)
    {
        delete metaVolume;
    }

protected:
    const int DEFAULT_SIZE = 5;

    MetaVolumeTester* metaVolume = nullptr;
    NiceMock<MockInodeManager>* inodeMgr = nullptr;
    NiceMock<MockCatalogManager>* catalogMgr = nullptr;
    NiceMock<MockMetaStorageSubsystem>* metaStorage = nullptr;

    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;
    MetaLpnType maxVolumePageNum = 1024;
};

TEST_F(MetaVolumeFixture, CheckStoreability)
{
    FileSizeType size = 0;
    MetaFilePropertySet set;

    EXPECT_EQ(metaVolume->IsOkayToStore(size, set), true);
}

TEST_F(MetaVolumeFixture, CreateVolumePositive)
{
    EXPECT_CALL(*catalogMgr, CreateCatalog).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, SaveContent).WillRepeatedly(Return(true));

    EXPECT_TRUE(metaVolume->CreateVolume());
}

TEST_F(MetaVolumeFixture, CreateVolumeNegative_DueTo_CreateCatalog)
{
    EXPECT_CALL(*catalogMgr, CreateCatalog).WillRepeatedly(Return(false));

    EXPECT_FALSE(metaVolume->CreateVolume());
}

TEST_F(MetaVolumeFixture, CreateVolumeNegative_DueTo_SaveContent_CatalogMgr)
{
    EXPECT_CALL(*catalogMgr, CreateCatalog).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, SaveContent).WillRepeatedly(Return(false));

    EXPECT_FALSE(metaVolume->CreateVolume());
}

TEST_F(MetaVolumeFixture, CreateVolumeNegative_DueTo_SaveContent_InodeMgr)
{
    EXPECT_CALL(*catalogMgr, CreateCatalog).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, SaveContent).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent).WillRepeatedly(Return(false));

    EXPECT_FALSE(metaVolume->CreateVolume());
}

TEST_F(MetaVolumeFixture, OpenVolumePositive)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadInodeContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();

    EXPECT_TRUE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumeNegative_DueTo_LoadVolCatalog)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(false));
    EXPECT_CALL(*catalogMgr, RestoreContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadInodeContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();

    EXPECT_FALSE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumeNegative_DueTo_LoadInodeContent)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadInodeContent()).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, RestoreContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();

    EXPECT_FALSE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumePositive_Even_If_RestoreContent_CatalogMgr)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_,_,_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadInodeContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();

    EXPECT_TRUE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumePositive_Even_If_RestoreContent_InodeMgr)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadInodeContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_,_,_,_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();

    EXPECT_TRUE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumeNegative_DueTo_RestoreContent_CatalogMgr)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_,_,_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadInodeContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    metaVolume->SetVolumeType(MetaVolumeType::NvRamVolume);

    EXPECT_FALSE(metaVolume->OpenVolume(info, true));
}

TEST_F(MetaVolumeFixture, OpenVolumeNegative_DueTo_RestoreContent_InodeMgr)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadInodeContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_,_,_,_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    metaVolume->SetVolumeType(MetaVolumeType::NvRamVolume);

    EXPECT_FALSE(metaVolume->OpenVolume(info, true));
}

TEST_F(MetaVolumeFixture, CloseVolumePositive)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);

    EXPECT_TRUE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumePositive_VolumeIsNotClosed)
{
    MetaLpnType* info = new MetaLpnType[2]();
    bool resetContext = false;

    EXPECT_TRUE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_GetFileCountInActive)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(1));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_SaveContent_CatalogMgr)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_SaveContent_InodeMgr)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(false));
    EXPECT_CALL(*catalogMgr, BackupContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_BackupContent_CatalogMgr)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_,_,_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, BackupContent(_,_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);
    metaVolume->SetVolumeType(MetaVolumeType::NvRamVolume);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_BackupContent_InodeMgr)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_,_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_,_,_,_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);
    metaVolume->SetVolumeType(MetaVolumeType::NvRamVolume);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CheckActiveFds)
{
    EXPECT_CALL(*inodeMgr, AddFileInActiveList).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));
    EXPECT_CALL(*inodeMgr, CheckFileInActive).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RemoveFileFromActiveList).WillRepeatedly(Return());

    for (FileDescriptorType fd = 0; fd < 5; ++fd)
    {
        EXPECT_EQ(metaVolume->AddFileInActiveList(fd), POS_EVENT_ID::SUCCESS);
    }

    for (FileDescriptorType fd = 0; fd < 5; ++fd)
    {
        EXPECT_TRUE(metaVolume->CheckFileInActive(fd));
    }

    for (FileDescriptorType fd = 0; fd < 5; ++fd)
    {
        metaVolume->RemoveFileFromActiveList(fd);
    }
}

TEST_F(MetaVolumeFixture, CheckBaseLpn)
{
    EXPECT_EQ(metaVolume->GetBaseLpn(), 5);
}

TEST_F(MetaVolumeFixture, CheckMaxLpn)
{
    EXPECT_EQ(metaVolume->GetMaxLpn(), 1024);
}
} // namespace pos
