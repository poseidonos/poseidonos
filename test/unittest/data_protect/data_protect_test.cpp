#include <iostream>
#include "e2e_protect.h"
#include <gtest/gtest.h>

using namespace ibofos;
using namespace std;


namespace
{

class DataProtectTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

public:
    static const size_t testSize = 4096;

    void MakeTestData(string &testData, size_t len)
    {
        static const char alphanum[] = 
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        
        for (size_t index = 0; index < len; ++index)
        {
            testData += alphanum[std::rand() % (sizeof(alphanum) - 1)];
        }
    }
};

TEST_F(DataProtectTest, MakeSameParityValueTest)
{
    string testData;
    string refData;
    uint32_t testParity = 0;
    uint32_t refParity = 0;

    DataProtect *dataProtect= new DataProtect();


    MakeTestData(refData, DataProtectTest::testSize);
    testData = refData;

    //refParity = dataProtect->MakeParity((void*)refData.c_str(),
    refParity = dataProtect->MakeParity((unsigned char*)refData.c_str(),
                                        DataProtectTest::testSize);
    
    //testParity = dataProtect->MakeParity((void*)testData.c_str(),
    testParity = dataProtect->MakeParity((unsigned char*)testData.c_str(),
                                         DataProtectTest::testSize);

    EXPECT_TRUE(refParity == testParity);

}

TEST_F(DataProtectTest, ParityValidateFailTest)
{
    string testData;
    string refData;
    uint32_t testParity = 0;
    uint32_t refParity = 0;

    DataProtect *dataProtect= new DataProtect();

    MakeTestData(refData, DataProtectTest::testSize);
    testData = refData;
    testData.replace(0, 1, "1");
    
    //refParity = dataProtect->MakeParity((void*)refData.c_str(),
    refParity = dataProtect->MakeParity((unsigned char*)refData.c_str(),
                                        DataProtectTest::testSize);
    //testParity = dataProtect->MakeParity((void*)testData.c_str(),
    testParity = dataProtect->MakeParity((unsigned char*)testData.c_str(),
                                         DataProtectTest::testSize);

    EXPECT_FALSE(refParity == testParity);


}
}

int main(int argc, char **argv)
{
    printf("running main ");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

