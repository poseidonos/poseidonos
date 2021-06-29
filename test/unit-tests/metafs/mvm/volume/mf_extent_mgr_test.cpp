#include "src/metafs/mvm/volume/mf_extent_mgr.h"

#include <gtest/gtest.h>

namespace pos
{
class MetaFileExtentManagerTester : public MetaFileExtentManager
{
public:
    MetaFileExtentManagerTester(void)
    {
    }

    virtual ~MetaFileExtentManagerTester(void)
    {
    }

    void MergeFreeExtents(void)
    {
        _MergeFreeExtents();
    }

    std::deque<MetaFileExtent>& GetFreeExtentsList(void)
    {
        return freeExtentsList;
    }

    void PrintFreeExtentList(void)
    {
        std::cout << "count of freeExtentsList: " << freeExtentsList.size() << std::endl;

        for (auto it : freeExtentsList)
        {
            std::cout << "{" << it.GetStartLpn() << ", " << it.GetCount() << "}" << std::endl;
        }
    }
};

TEST(MetaFileExtentManager, AllocExtent_runCorrectly)
{
    // Given
    MetaFileExtentManager extentMgr;

    // When
    extentMgr.Init(0, 100); // start lpn:0, last lpn: 100, count: 101
    MetaLpnType availableLpn = extentMgr.GetAvailableLpnCount();
    MetaLpnType biggestLpn = extentMgr.GetTheBiggestExtentSize();

    // Then
    EXPECT_EQ(availableLpn, 101);
    EXPECT_EQ(biggestLpn, 101);

    // When
    MetaFilePageMap map = extentMgr.AllocExtent(100);

    // Then
    EXPECT_EQ(map.baseMetaLpn, 0);
    EXPECT_EQ(map.pageCnt, 100);

    // When
    extentMgr.SetFileBaseLpn(1);
    availableLpn = extentMgr.GetAvailableLpnCount();

    // Then
    EXPECT_EQ(availableLpn, 100);

    // When
    MetaLpnType baseLpn = extentMgr.GetFileBaseLpn();

    // Then
    EXPECT_EQ(baseLpn, 1);
}

TEST(MetaFileExtentManager, Control_Contents)
{
    // Given
    MetaFileExtentManager extentMgr;
    extentMgr.Init(0, 100); // start lpn:0, last lpn: 100, count: 101

    MetaFileExtent list[MetaFsConfig::MAX_VOLUME_CNT];
    MetaFileExtent list_temp[MetaFsConfig::MAX_VOLUME_CNT];
    memset(list, 0x0, sizeof(MetaFileExtent) * MetaFsConfig::MAX_VOLUME_CNT);
    memset(list_temp, 0x0, sizeof(MetaFileExtent) * MetaFsConfig::MAX_VOLUME_CNT);

    // (0, 19) (20, 5) (25, 4) - free count 21 - (50, 9) - free count 42 -
    list[0].SetStartLpn(0);
    list[0].SetCount(19);
    list[1].SetStartLpn(20);
    list[1].SetCount(5);
    list[2].SetStartLpn(25);
    list[2].SetCount(4);
    list[3].SetStartLpn(50);
    list[3].SetCount(9);

    // When
    extentMgr.SetContent(list);
    extentMgr.GetContent(list_temp);

    // Then
    MetaLpnType biggestLpn = extentMgr.GetTheBiggestExtentSize();
    EXPECT_EQ(biggestLpn, 42);

    // Then
    int usedLpnCount = 0;
    int usedEntryCount = 0;

    for (int index = 0; index < MetaFsConfig::MAX_VOLUME_CNT; index++)
    {
        usedLpnCount += list_temp[index].GetCount();
        if (list_temp[index].GetCount() != 0)
            usedEntryCount++;
    }
    EXPECT_EQ(usedLpnCount, 37);
    EXPECT_EQ(usedEntryCount, 3);
}

TEST(MetaFileExtentManager, MergeFreeExtents)
{
    // Given
    MetaFileExtentManagerTester extentMgr;

    // When
    std::deque<MetaFileExtent>& freeList = extentMgr.GetFreeExtentsList();
    freeList.emplace_back(MetaFileExtent {0, 4});
    freeList.emplace_back(MetaFileExtent {5, 5});
    freeList.emplace_back(MetaFileExtent {10, 4});
    freeList.emplace_back(MetaFileExtent {15, 4});
    freeList.emplace_back(MetaFileExtent {20, 4});

    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 5);

    extentMgr.MergeFreeExtents();

    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 4);
}

} // namespace pos
