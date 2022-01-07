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

#include "src/metafs/common/meta_file_name.h"

#include <gtest/gtest.h>
#include <string>

namespace pos
{
TEST(MetaFileName, ConstructObjectWithNullptr)
{
    EXPECT_DEATH(MetaFileName(nullptr), "");
}

TEST(MetaFileName, ConstructObjectWithTooLongString)
{
    const size_t MAX_SIZE = MetaFileName::MAX_FILE_NAME_LENGTH;
    char fileName[MetaFileName::MAX_FILE_NAME_LENGTH + 5] = { 0, };

    // length == 127
    for (size_t i = 0; i < MetaFileName::MAX_FILE_NAME_LENGTH; i++)
        fileName[i] = 'A';

    MetaFileName name = std::string(fileName);
    ASSERT_EQ(name.GetLength(), MAX_SIZE);

    // length == 128
    fileName[127] = 'A';
    EXPECT_DEATH(MetaFileName(std::string(fileName)), "") << "fileName.length(): " << std::string(fileName).length();
}

TEST(MetaFileName, Equal)
{
    const std::string fileName = "ThisIsTestFileName";

    // construct by reference and ptr
    EXPECT_EQ(MetaFileName(fileName).ToString(), fileName);
    EXPECT_EQ(MetaFileName(&fileName).ToString(), fileName);

    // copy constructor
    MetaFileName name(fileName);
    MetaFileName name2 = name;

    EXPECT_EQ(name.ToString(), name2.ToString());
    EXPECT_EQ(name.GetLength(), name2.GetLength());
    EXPECT_EQ(0, name.ToString().compare(name2.ToString()));
    // check all chars include null
    for (size_t i = 0; i <= name.GetLength(); ++i)
        EXPECT_EQ(name.ToChar()[i], name2.ToChar()[i]);

    // changed the first object only
    name = "TestFile";
    EXPECT_NE(0, name.ToString().compare(name2.ToString()));
}

TEST(MetaFileName, ToString)
{
    const std::string fileName = "ThisIsTestFileName";
    EXPECT_TRUE(MetaFileName(fileName).ToString() == fileName);
}

TEST(MetaFileName, ToChar)
{
    const std::string fileName = "ThisIsTestFileName";
    bool result = true;

    MetaFileName obj(fileName);

    for (int i = 0; i < fileName.size(); ++i)
    {
        if (fileName.c_str()[i] != obj.ToChar()[i])
        {
            result = false;
            break;
        }
    }

    EXPECT_TRUE(result);
}

TEST(MetaFileName, CheckAssignment)
{
    const std::string fileName = "ThisIsTestFileName";

    MetaFileName name;
    EXPECT_EQ(0, name.GetLength());

    name = fileName;
    EXPECT_EQ(fileName.length(), name.GetLength());

    name = "";
    EXPECT_EQ(0, name.GetLength());

    name = &fileName;
    EXPECT_EQ(fileName.length(), name.GetLength());
}

TEST(MetaFileName, CompareWithString)
{
    const std::string fileName = "ThisIsTestFileName";
    MetaFileName name;

    EXPECT_FALSE(name == fileName);
    EXPECT_TRUE(name != fileName);

    name = fileName;

    EXPECT_TRUE(name == fileName);
    EXPECT_FALSE(name != fileName);
}

TEST(MetaFileName, CompareWithFileNameObject)
{
    const std::string fileName = "ThisIsTestFileName";
    MetaFileName name;
    MetaFileName name2 = fileName;

    EXPECT_FALSE(name == name2);
    EXPECT_TRUE(name != name2);

    name = fileName;

    EXPECT_TRUE(name == name2);
    EXPECT_FALSE(name != name2);
}
} // namespace pos
