/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/metafs/mvm/volume/extent_allocator.h"

#include <gtest/gtest.h>

namespace pos
{
class ExtentAllocatorTester : public ExtentAllocator
{
public:
    ExtentAllocatorTester(void)
    {
    }

    virtual ~ExtentAllocatorTester(void)
    {
    }

    std::deque<MetaFileExtent>& GetFreeList(void)
    {
        return freeList;
    }

    void PrintFreeExtentList(void)
    {
        std::cout << "count of freeExtentsList: " << freeList.size() << std::endl;

        for (auto it : freeList)
        {
            std::cout << "{" << it.GetStartLpn() << ", " << it.GetCount() << "}" << std::endl;
        }
    }
};

TEST(ExtentAllocator, AllocExtent_runCorrectly)
{
    // Given
    ExtentAllocator extentMgr;

    // When
    // start lpn:0, last lpn: 100, available count: 101
    extentMgr.Init(0, 100);
    MetaLpnType availableLpn = extentMgr.GetAvailableLpnCount();

    // Then
    EXPECT_EQ(availableLpn, 101);

    // When
    // start lpn:1, last lpn: 100, available count: 100
    extentMgr.SetFileBaseLpn(1);
    // start lpn:1, last lpn: 100, available count: 20
    std::vector<MetaFileExtent> map = extentMgr.AllocExtents(80);
    availableLpn = extentMgr.GetAvailableLpnCount();

    // Then
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map[0].GetStartLpn(), 1);
    EXPECT_EQ(map[0].GetCount(), 80);

    EXPECT_EQ(availableLpn, 20);

    // When
    MetaLpnType baseLpn = extentMgr.GetFileBaseLpn();

    // Then
    EXPECT_EQ(baseLpn, 1);
}

TEST(ExtentAllocator, Control_Contents)
{
    // Given
    ExtentAllocator extentMgr;
    extentMgr.Init(0, 100); // start lpn:0, last lpn: 100, count: 101

    std::vector<MetaFileExtent> list;
    std::vector<MetaFileExtent> list_temp;

    // (0, 19) (20, 5) (25, 4) - free count 22 - (50, 9) - free count 42 -
    list.push_back({0, 19});
    list.push_back({20, 5});
    list.push_back({25, 4});
    list.push_back({50, 9});

    // When
    extentMgr.SetAllocatedExtentList(list);
    list_temp = extentMgr.GetAllocatedExtentList();

    // Then
    int usedLpnCount = 0;
    int usedEntryCount = 0;

    for (int index = 0; index < (int)list_temp.size(); index++)
    {
        usedLpnCount += list_temp[index].GetCount();
        if (list_temp[index].GetCount() != 0)
            usedEntryCount++;
    }

    extentMgr.PrintFreeExtentsList();

    EXPECT_EQ(usedLpnCount, 37);
    EXPECT_EQ(usedEntryCount, 3);
}

TEST(ExtentAllocator, AllocExtent_Simple_Positive0)
{
    // Given
    ExtentAllocatorTester extentMgr;
    extentMgr.Init(0, 49);

    std::vector<MetaFileExtent> map = extentMgr.AllocExtents(47);

    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map[0].GetStartLpn(), 0);
    EXPECT_EQ(map[0].GetCount(), 48);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 2);
}

TEST(ExtentAllocator, AllocExtent_Simple_Positive1)
{
    // Given
    ExtentAllocatorTester extentMgr;
    extentMgr.Init(0, 49);

    std::vector<MetaFileExtent> map = extentMgr.AllocExtents(48);

    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map[0].GetStartLpn(), 0);
    EXPECT_EQ(map[0].GetCount(), 48);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 2);
}

TEST(ExtentAllocator, AllocExtent_Simple_Negative)
{
    // Given
    ExtentAllocatorTester extentMgr;
    extentMgr.Init(0, 49);

    std::vector<MetaFileExtent> map = extentMgr.AllocExtents(49);

    EXPECT_EQ(map.size(), 0);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 50);
}

TEST(ExtentAllocator, Range1)
{
    // Given
    ExtentAllocatorTester extentMgr;
    extentMgr.Init(0, 49);

    std::vector<MetaFileExtent> map = extentMgr.AllocExtents(46);

    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map[0].GetStartLpn(), 0);
    EXPECT_EQ(map[0].GetCount(), 48);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 2);

    map = extentMgr.AllocExtents(2);
    EXPECT_EQ(map.size(), 0);

    // When
    std::deque<MetaFileExtent>& freeList = extentMgr.GetFreeList();
    extentMgr.AddToFreeList(0, 8);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 10);
    extentMgr.AddToFreeList(16, 8);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 18);
    extentMgr.AddToFreeList(32, 8);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 26);

    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 4);

    MetaLpnType size = 0;
    for (int i = 0; i < freeList.size(); ++i)
    {
        size += freeList[i].GetCount();
    }

    extentMgr.AddToFreeList(40, 8);
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 3);

    MetaLpnType size2 = 0;
    for (int i = 0; i < freeList.size(); ++i)
    {
        size2 += freeList[i].GetCount();
    }

    EXPECT_EQ(size + 8, size2);
}

TEST(ExtentAllocator, Range2)
{
    // Given
    ExtentAllocatorTester extentMgr;
    extentMgr.Init(0, 99);

    std::deque<MetaFileExtent>& freeList = extentMgr.GetFreeList();
    std::vector<MetaFileExtent> map[7];
    map[0] = extentMgr.AllocExtents(10);
    EXPECT_EQ(map[0].size(), 1);
    EXPECT_EQ(map[0][0].GetStartLpn(), 0);
    EXPECT_EQ(map[0][0].GetCount(), 16);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 84);

    map[1] = extentMgr.AllocExtents(5);
    EXPECT_EQ(map[1].size(), 1);
    EXPECT_EQ(map[1][0].GetStartLpn(), 16);
    EXPECT_EQ(map[1][0].GetCount(), 8);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 76);

    map[2] = extentMgr.AllocExtents(5);
    EXPECT_EQ(map[2].size(), 1);
    EXPECT_EQ(map[2][0].GetStartLpn(), 24);
    EXPECT_EQ(map[2][0].GetCount(), 8);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 68);

    map[3] = extentMgr.AllocExtents(5);
    EXPECT_EQ(map[3].size(), 1);
    EXPECT_EQ(map[3][0].GetStartLpn(), 32);
    EXPECT_EQ(map[3][0].GetCount(), 8);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 60);

    map[4] = extentMgr.AllocExtents(5);
    EXPECT_EQ(map[4].size(), 1);
    EXPECT_EQ(map[4][0].GetStartLpn(), 40);
    EXPECT_EQ(map[4][0].GetCount(), 8);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 52);

    map[5] = extentMgr.AllocExtents(10);
    EXPECT_EQ(map[5].size(), 1);
    EXPECT_EQ(map[5][0].GetStartLpn(), 48);
    EXPECT_EQ(map[5][0].GetCount(), 16);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 36);

    map[6] = extentMgr.AllocExtents(5);
    EXPECT_EQ(map[6].size(), 1);
    EXPECT_EQ(map[6][0].GetStartLpn(), 64);
    EXPECT_EQ(map[6][0].GetCount(), 8);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 28);

    EXPECT_EQ(freeList.size(), 1);

    extentMgr.AddToFreeList(map[2][0].GetStartLpn(), map[2][0].GetCount());
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 2);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 36);

    extentMgr.AddToFreeList(map[4][0].GetStartLpn(), map[4][0].GetCount());
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 3);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 44);

    extentMgr.AddToFreeList(map[0][0].GetStartLpn(), map[0][0].GetCount());
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 4);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 60);

    extentMgr.AddToFreeList(map[1][0].GetStartLpn(), map[1][0].GetCount());
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 3);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 68);

    extentMgr.AddToFreeList(map[6][0].GetStartLpn(), map[6][0].GetCount());
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 3);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 76);

    extentMgr.AddToFreeList(map[3][0].GetStartLpn(), map[3][0].GetCount());
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 2);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 84);

    extentMgr.AddToFreeList(map[5][0].GetStartLpn(), map[5][0].GetCount());
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 1);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 100);
}

TEST(ExtentAllocator, Range3)
{
    // Given
    ExtentAllocatorTester extentMgr;
    extentMgr.Init(0, 99);

    std::deque<MetaFileExtent>& freeList = extentMgr.GetFreeList();
    // available: 28
    std::vector<MetaFileExtent> map = extentMgr.AllocExtents(72);

    // available: 36
    extentMgr.AddToFreeList(16, 8);
    // available: 44
    extentMgr.AddToFreeList(32, 8);
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(freeList.size(), 3);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 44);

    // available: 4
    map = extentMgr.AllocExtents(17);

    EXPECT_EQ(map.size(), 3);
    if (map.size() == 3)
    {
        EXPECT_EQ(map[0].GetStartLpn(), 16);
        EXPECT_EQ(map[0].GetCount(), 8);
        EXPECT_EQ(map[1].GetStartLpn(), 32);
        EXPECT_EQ(map[1].GetCount(), 8);
        EXPECT_EQ(map[2].GetStartLpn(), 72);
        EXPECT_EQ(map[2].GetCount(), 8);

        extentMgr.PrintFreeExtentList();
        EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 20);

        extentMgr.AddToFreeList(map[0].GetStartLpn(), map[0].GetCount());
        extentMgr.AddToFreeList(map[1].GetStartLpn(), map[1].GetCount());
        extentMgr.AddToFreeList(map[2].GetStartLpn(), map[2].GetCount());

        extentMgr.PrintFreeExtentList();
        EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 44);
    }

    map = extentMgr.AllocExtents(16);

    EXPECT_EQ(map.size(), 2);
    if (map.size() == 2)
    {
        EXPECT_EQ(map[0].GetStartLpn(), 16);
        EXPECT_EQ(map[0].GetCount(), 8);
        EXPECT_EQ(map[1].GetStartLpn(), 32);
        EXPECT_EQ(map[1].GetCount(), 8);

        extentMgr.PrintFreeExtentList();
        EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 28);

        extentMgr.AddToFreeList(map[0].GetStartLpn(), map[0].GetCount());
        extentMgr.AddToFreeList(map[1].GetStartLpn(), map[1].GetCount());

        extentMgr.PrintFreeExtentList();
        EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 44);
    }
}

TEST(ExtentAllocator, Range4)
{
    // Given
    ExtentAllocatorTester extentMgr;
    extentMgr.Init(0, 10000);
    extentMgr.SetFileBaseLpn(4051);

    std::deque<MetaFileExtent>& freeList = extentMgr.GetFreeList();
    std::vector<MetaFileExtent> map = extentMgr.AllocExtents(5000);

    // init state
    std::cout << "init state" << std::endl;
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map[0].GetStartLpn(), 4051);
    EXPECT_EQ(map[0].GetCount(), 5000);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 950);

    // release lpns
    std::cout << "release 800 lpns" << std::endl;
    extentMgr.AddToFreeList(4155, 80);
    extentMgr.AddToFreeList(4315, 80);
    extentMgr.AddToFreeList(4475, 80);
    extentMgr.AddToFreeList(4635, 80);
    extentMgr.AddToFreeList(4795, 80);
    extentMgr.AddToFreeList(4955, 80);
    extentMgr.AddToFreeList(5275, 80);
    extentMgr.AddToFreeList(5595, 80);
    extentMgr.AddToFreeList(7355, 80);
    extentMgr.AddToFreeList(8315, 80);
    extentMgr.PrintFreeExtentList();
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 1750);

    // fail to get extents, 1745 remained but less than 1752, a multiple of 8
    map = extentMgr.AllocExtents(1745);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 1750);
    EXPECT_EQ(map.size(), 0);

    // success to get extents, 1750 remained
    map = extentMgr.AllocExtents(1744);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 6);
    EXPECT_EQ(map.size(), 11);

    std::cout << "allocated info" << std::endl;
    for (int i = 0; i < (int)map.size(); ++i)
    {
        std::cout << i << " {";
        std::cout << map[i].GetStartLpn() << ", " << map[i].GetCount();
        std::cout << "}" << std::endl;
    }

    // fail to get extents, 6 remained but less than 8, a multiple of 8
    map = extentMgr.AllocExtents(4);
    EXPECT_EQ(map.size(), 0);

    extentMgr.PrintFreeExtentList();

    // release
    extentMgr.AddToFreeList(4235, 80);
    extentMgr.AddToFreeList(4555, 80);
    extentMgr.AddToFreeList(5515, 80);
    extentMgr.AddToFreeList(5915, 80);
    extentMgr.AddToFreeList(6315, 80);
    EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 406);

    extentMgr.PrintFreeExtentList();

    // repeat to allocate and release
    for (int i = 0; i < 10; ++i)
    {
        std::cout << "test loop: " << i << std::endl;

        // alloc
        map = extentMgr.AllocExtents(300);
        EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 102);

        // release
        for (auto& extent : map)
        {
            extentMgr.AddToFreeList(extent.GetStartLpn(), extent.GetCount());
        }
        map.clear();
        EXPECT_EQ(extentMgr.GetAvailableLpnCount(), 406);
    }
}
} // namespace pos
