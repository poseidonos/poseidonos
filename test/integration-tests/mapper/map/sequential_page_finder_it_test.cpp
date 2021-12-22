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

#include "test/integration-tests/mapper/map/sequential_page_finder_it_test.h"

#include <list>

namespace pos
{
void
SequentialPageFinderTest::SetUp(void)
{
}

void
SequentialPageFinderTest::TearDown(void)
{
}

void
SequentialPageFinderTest::_AddPages(MpageSet set, MpageList& dest)
{
    for (int currentPage = set.startMpage;
        currentPage < set.startMpage + set.numMpages; currentPage++)
    {
        dest.insert(currentPage);
    }
}

TEST_F(SequentialPageFinderTest, SequentialPages)
{
    MpageList pages;
    for (int pageNum = 0; pageNum < 100; pageNum++)
    {
        pages.insert(pageNum);
    }

    SequentialPageFinder finder(pages);
    MpageSet currentSet = finder.PopNextMpageSet();
    EXPECT_TRUE(currentSet.numMpages == 1);
    EXPECT_TRUE(currentSet.startMpage == 0);

    EXPECT_TRUE(finder.IsRemaining() == true);
}

TEST_F(SequentialPageFinderTest, SequentialPagesWithBitmap)
{
    BitMap* bitmap = new BitMap(100);
    for (int pageNum = 0; pageNum < 100; pageNum++)
    {
        bitmap->SetBit(pageNum);
    }

    SequentialPageFinder finder(bitmap);
    MpageSet currentSet = finder.PopNextMpageSet();
    EXPECT_TRUE(currentSet.numMpages == 1);
    EXPECT_TRUE(currentSet.startMpage == 0);

    EXPECT_TRUE(finder.IsRemaining() == true);
}

TEST_F(SequentialPageFinderTest, LargeSequentialPages)
{
    MpageList pages;
    int numPagesToTest = 4000;
    for (int pageNum = 0; pageNum < numPagesToTest; pageNum++)
    {
        pages.insert(pageNum);
    }

    SequentialPageFinder finder(pages);

    for (int count = 0; count < numPagesToTest / MAX_MPAGES_PER_SET; count++)
    {
        MpageSet currentSet = finder.PopNextMpageSet();
        EXPECT_TRUE(currentSet.numMpages == MAX_MPAGES_PER_SET);
        EXPECT_TRUE(currentSet.startMpage == MAX_MPAGES_PER_SET * count);
    }
    if (numPagesToTest % MAX_MPAGES_PER_SET != 0)
    {
        MpageSet currentSet = finder.PopNextMpageSet();
        EXPECT_TRUE(currentSet.numMpages == numPagesToTest % MAX_MPAGES_PER_SET);
        EXPECT_TRUE(currentSet.startMpage == (numPagesToTest / MAX_MPAGES_PER_SET) * MAX_MPAGES_PER_SET);
    }

    EXPECT_TRUE(finder.IsRemaining() == false);
}

TEST_F(SequentialPageFinderTest, SeveralSequentialPages)
{
    std::list<MpageSet> sets;

    int maxInterval = 100;

    int currentPage = 0;
    int numSetsToTest = 5;

    for (int count = 0; count < numSetsToTest; count++)
    {
        MpageSet currentSet;
        currentSet.startMpage = currentPage;
        currentSet.numMpages = std::rand() % (MAX_MPAGES_PER_SET - 1) + 1;

        currentPage += currentSet.numMpages;
        currentPage += (std::rand() % (maxInterval - 1) + 1);

        sets.push_back(currentSet);
    }

    MpageList pagesToTest;
    for (auto s : sets)
    {
        _AddPages(s, pagesToTest);
    }

    SequentialPageFinder finder(pagesToTest);

    EXPECT_TRUE(finder.IsRemaining() == true);

    while (finder.IsRemaining() == true)
    {
        MpageSet read = finder.PopNextMpageSet();
        MpageSet expect = sets.front();

        EXPECT_TRUE(read.startMpage == expect.startMpage);
        EXPECT_TRUE(read.numMpages == expect.numMpages);

        sets.pop_front();
    }

    EXPECT_TRUE(sets.size() == 0);
}

}   // namespace pos
