#include "src/metafs/common/meta_file_name.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFileName, Equal)
{
    const std::string fileName = "ThisIsTestFileName";
    bool result = true;

    MetaFileName obj;
    obj = &fileName;

    for (int i = 0; i < fileName.size(); ++i)
    {
        if (fileName.c_str()[i] != obj.str[i])
        {
            result = false;
            break;
        }
    }

    EXPECT_TRUE(result);
}

TEST(MetaFileName, ToString)
{
    const std::string fileName = "ThisIsTestFileName";
    bool result = true;

    MetaFileName obj;
    obj = &fileName;

    string test = obj.ToString();

    if (test != fileName)
        result = false;

    EXPECT_TRUE(result);
}

TEST(MetaFileName, ToChar)
{
    const std::string fileName = "ThisIsTestFileName";
    bool result = true;

    MetaFileName obj;
    obj = &fileName;

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

} // namespace pos
